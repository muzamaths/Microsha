#ifndef MICROSHA_COMMAND_PIPELINE_H
#define MICROSHA_COMMAND_PIPELINE_H

#include <string>
#include <vector>
#include <deque>

#include "command.h"

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
  =default;

  /* Reset pipeline */
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
   * Note: Function does not check if 'command_line' parameter fits 'splitted_command_line' parameter. */
  ERR_CODE check_cmd_line_IO_pattern(const std::string &command_line, const std::vector<std::string> &splitted_command_line)
  {
    if (command_line.empty())
    {
      return FAILURE;
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

  }

};

#endif //MICROSHA_COMMAND_PIPELINE_H
