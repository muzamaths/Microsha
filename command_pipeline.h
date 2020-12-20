#ifndef MICROSHA_COMMAND_PIPELINE_H
#define MICROSHA_COMMAND_PIPELINE_H

#include <fcntl.h>
#include <sys/times.h>
#include <sys/wait.h>

#include <string>
#include <vector>
#include <deque>
#include <iomanip>

#include "command.h"

#define READ_END 0
#define WRITE_END 1

/* Class obtaining command pipeline
 * Pipeline have following pattern : command_1 (< is) | command_2 | ... | command_k (> os) -
 * only first command can have external input stream, and only last command can have external output */
class command_pipeline
{
private:
  std::deque<command> command_queue;

public:
  /* Default class constructor */
  command_pipeline()
  =default;

  /* Default class destructor */
  ~command_pipeline()
  {
    clear_pipeline();
  }

  /* Reset pipeline
   * Note : if command_line is empty 'SUCCESS' is returned */
  ERR_CODE reset_pipeline(const std::string &command_line)
  {
    clear_pipeline();

    std::vector<std::string> splitted_cmd_line;
    split_string_by_token(command_line, '|', splitted_cmd_line);

    // check if number of external i\o ( </> ) points fits the pattern in the description of class
    if (check_cmd_line_IO_pattern(command_line, splitted_cmd_line) != SUCCESS)
    {
      ADD_LOG_WITH_RETURN(ERR_WRONG_INPUT, 2);
    }

    // insert all command bases into deque. "command" class object are constructed on-place
    for(auto &curr_cmd_base : splitted_cmd_line)
    {
      ERR_CODE err_code = SUCCESS; // TODO: ask if it is optimized by compiler and is it OK to write it here
      command_queue.emplace_back(curr_cmd_base, err_code);

      if (err_code != SUCCESS)
      {
        clear_pipeline();
        ADD_LOG_WITH_RETURN(err_code, 0);
      }
    }

    return SUCCESS;
  }

  /* Checks if command line fits the IO pattern(<,> position and number) in the description of class.
   * Note:
   * - function does not check if 'command_line' parameter fits 'splitted_command_line' parameter;
   * - if command_line is empty 'SUCCESS' is returned. */
  ERR_CODE check_cmd_line_IO_pattern(const std::string &command_line, const std::vector<std::string> &splitted_command_line)
  {
    if (command_line.empty())
    {
      return SUCCESS;
    }

    auto
      input_points_num  = std::count(command_line.begin(), command_line.end(), '<'),
      output_points_num = std::count(command_line.begin(), command_line.end(), '>'),
      input_points_num_in_front = std::count(splitted_command_line.front().begin(), splitted_command_line.front().end(), '<'),
      output_points_num_in_back = std::count(splitted_command_line.back().begin() , splitted_command_line.back().end() , '>');

    if (!(input_points_num  == input_points_num_in_front && input_points_num  <= 1 &&
          output_points_num == output_points_num_in_back && output_points_num <= 1))
    {
      return FAILURE;
    }

    return SUCCESS;
  }

  /* Clear pipeline */
  void clear_pipeline()
  {
    command_queue.clear();
  }

  /* Obtains command pipeline work */
  ERR_CODE exec()
  {
    // empty command queue obtain
    if (command_queue.empty())
    {
      return SUCCESS;
    }

    // obtain time command
    if (command_queue.front().cmd_type == CMD_TIME)
    {
      IS_SUCCESS_WITH_RETURN(exec_with_time())
      return SUCCESS;
    }

    // execute pipeline
    auto &front_cmd = command_queue.front();

    if (front_cmd.cmd_type == CMD_CD) // it has no output information and can not be part of pipeline
    {
      IS_SUCCESS_WITH_RETURN(front_cmd.exec())
      return SUCCESS;
    }

    // creating pipes for pipeline. TODO : explore pipe work and may be ask how to make it work with only one pipe
    std::vector<int[2]> pipe_array(command_queue.size() - 1);
    for (auto &s : pipe_array)
    {
      if (pipe2(s, O_CLOEXEC) != 0)
      {
        std::cerr << "Can not open pipe\n";
        ADD_LOG_WITH_RETURN(FAILURE, 3);
      }
    }

    // connect created pipes so as they constitute pipeline, create child process and execute command in it
    for (int i = 0; i < command_queue.size(); i++)
    {
      pid_t pid = fork();

      if (pid == 0) // child
      {
        if (command_queue.size() == 1) // no pipeline is required, that is why no pipes were create. just exec one cmd
        {
          command_queue.front().exec();
        }
        else if (i == 0)
        {
          dup2(pipe_array[i][WRITE_END], STDOUT_FILENO);
          close(pipe_array[i][READ_END]);
          command_queue[i].exec();
        }
        else if (i > 0 && i < (command_queue.size() - 1))
        {
          dup2(pipe_array[i - 1][READ_END], STDIN_FILENO);
          dup2(pipe_array[i][WRITE_END], STDOUT_FILENO);

          close(pipe_array[i - 1][WRITE_END]);
          close(pipe_array[i][READ_END]);
        }
        else // if (i == (command_queue.size() - 1))
        {
          dup2(pipe_array[i - 1][READ_END], STDIN_FILENO);
          close(pipe_array[i - 1][WRITE_END]);
        }
      }
      else // if (pid != 0) - parent
      {
        if (i == (command_queue.size() - 1))
        {
          // close pipes
          for(auto &pipe : pipe_array)
          {
            // TODO : pipes correct close obtain
            close(pipe[READ_END]);
            close(pipe[WRITE_END]);
          }

          // collect all child processes end
          for (int j = 0; j < command_queue.size(); j++)
          {
            wait(nullptr);
          }
        }
      }
    }

    return SUCCESS;
  }

  /* Initiates pipeline execution removing measuring time and removing 'time' command from command queue
   * Note : function does not obtain situation when time containing variable are overfilled and so on.
   * TODO : ask is function should do queue empty-check and check if first element of command name is 'time' */
  ERR_CODE exec_with_time()
  {
    if (command_queue.empty() || command_queue.front().cmd_type != CMD_TIME)
    {
      return ERR_WRONG_INPUT;
    }

    // remove 'time' mark from the first command of the queue front
    auto &front_command = command_queue.front();
    front_command.command_name.erase(front_command.command_name.begin());
    front_command.cmd_type = (command_queue.empty()) ? CMD_OUT : command::get_command_type(front_command.command_name[0]);

    // initiate execution of left pipeline commands with time check
    auto cps = sysconf(_SC_CLK_TCK); // clicks per second
    tms start{}, stop{};
    clock_t start_real_time = times(&start);
    exec();
    clock_t stop_real_time = times(&stop);

    // print time values
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(3)
      << "real : " << get_time_in_sec(stop_real_time - start_real_time, cps) << "s" << std::endl
      << "user : " << get_time_in_sec((stop.tms_utime + stop.tms_cutime) - (start.tms_utime + start.tms_cutime), cps) << "s" << std::endl
      << "sys  : " << get_time_in_sec((stop.tms_stime + stop.tms_cstime) - (start.tms_stime + start.tms_cstime), cps) << "s" << std::endl;
    std::cout.unsetf(std::ios::fixed);

    return SUCCESS;
  }

  /* Returns time between 'start' and 'stop', which are given in clock_t, in seconds */
  static long double get_time_in_sec(clock_t time, long clocks_per_second)
  {
    return (long double)time / clocks_per_second;
  }
};

#endif //MICROSHA_COMMAND_PIPELINE_H
