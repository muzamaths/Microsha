#ifndef MICROSHA_COMMAND_H
#define MICROSHA_COMMAND_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <climits>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <dirent.h>

#include <vector>
#include <string>
#include <algorithm>

#include "string_funcitons.h"
#include "matcher.h"
#include "text_colors.h"

/* Enumeration for internal commands */
enum command_type
{
  CMD_OUT,  // either external command, or not command
  CMD_CD,   // changes directory
  CMD_PWD,  // shows present working directory
  CMD_TIME, // measures command work-time
  CMD_SET   // shows all shell-variables and environment variables
};

/* Command obtaining class */
class command
{
  friend class command_pipeline;

private:
  std::string input_file_name,
    output_file_name;
  std::vector<std::string> command_name;
  command_type cmd_type = CMD_OUT;

public:
  /* Default class constructor */
  command()
  =default;

  /* Default class destructor */
  ~command()
  =default;

  /* Class constructor by string.
   *
   * @param command_base - string containing command and attributes to be parsed
   * @param error_code   - reference to the variable where construction errors are to be written or 'SUCCESS' otherwise */
  command(const std::string &command_base, ERR_CODE &error_code)
  {
    error_code = parse_command(command_base);

    if (error_code == SUCCESS)
    {
      error_code = expand_command_path_params();
    }
  }

  /* Parses command string base */
  ERR_CODE parse_command(const std::string &command_base)
  {
    if (command_base.empty())
    {
      print_err(std::cerr, ERR_WRONG_INPUT);
      return ERR_WRONG_INPUT;
    }

    std::vector<std::string> command_parts;
    split_string_by_token(command_base, ' ', command_parts);

    // count number of i/o points TODO: ask isn't it excessive
    auto input_point_num  = std::count(command_parts.begin(), command_parts.end(), "<"),
         output_point_num = std::count(command_parts.begin(), command_parts.end(), ">");

    if (input_point_num > 1 || output_point_num > 1)
    {
      print_err(std::cerr, ERR_WRONG_INPUT);
      ADD_LOG_WITH_RETURN(ERR_WRONG_INPUT, 0);
    }

    // initialize command name and i/o file names
    auto input_point  = std::find(command_parts.begin(), command_parts.end(), "<"),
         output_point = std::find(command_parts.begin(), command_parts.end(), ">");

    if (output_point < input_point)
    {
      command_name.assign(command_parts.begin(), output_point);
      output_file_name = *(output_point + 1);
      if (input_point != command_parts.end()) // TODO: add format check : "cat <" or "< lol.txt" are not detected without error now
      {
        input_file_name = *(input_point + 1); // invalid pointer can be tried to go by
      }
    }
    else if (input_point < output_point)
    {
      command_name.assign(command_parts.begin(), input_point);
      input_file_name = *(input_point + 1);
      if (output_point != command_parts.end())
      {
        output_file_name = *(output_point + 1);
      }
    }
    else //if (input_point == output_point) -> input_point == command_parts.end() && output_point == command_parts.end()
    {
      command_name = command_parts;
    }

    if (!command_parts.empty())
    {
      cmd_type = get_command_type(command_parts[0]);
    }

    return SUCCESS;
  }

  /* Returns 'command_type' value by string */
  static command_type get_command_type(const std::string &cmd_name)
  {
    if      (cmd_name.empty()  ) { return CMD_OUT;  }
    else if (cmd_name == "cd"  ) { return CMD_CD;   }
    else if (cmd_name == "pwd" ) { return CMD_PWD;  }
    else if (cmd_name == "time") { return CMD_TIME; }
    else if (cmd_name == "set" ) { return CMD_SET;  }
    else                         { return CMD_OUT;  }
  }

  /**********************************************************************
   * Path regular expression expansion functions
   **********************************************************************/

  /* Checks if given text contains regex meta-symbols to be expanded */
  static bool is_expansion_needed(const std::string &text)
  {
    if (text.find('*') != std::string::npos || text.find('?') != std::string::npos)
    {
      return true;
    }
    return false;
  }

  /* Expands all path-regex parameters of command */
  ERR_CODE expand_command_path_params ()
  {
    for (int i = 0; i < command_name.size(); i++)
    {
      if (!is_expansion_needed(command_name[i])) { continue; }

      std::vector<std::string> expanded_path_names = expand_path_regex(command_name[i]);

      if (expanded_path_names.empty()) { continue; }

      command_name.erase(command_name.begin() + i);
      command_name.insert(command_name.begin() + i, expanded_path_names.begin(), expanded_path_names.end());
    }
    
    return SUCCESS;
  }
  
  /* Expands filesystem path regex */
  static std::vector<std::string> expand_path_regex(const std::string &path_regex)
  {
    std::vector<std::string> splitted_path_regex;
    split_string_by_token(path_regex, '/', splitted_path_regex);

    std::vector<std::string> expanded_path_name;
    std::string path;
    bool flag = false;

    if (splitted_path_regex[0].empty())
    {
      path = "/";
      flag = true;
    }
    else
    {
      path = get_curr_dir() + "/";
    }
    expanded_path_name.emplace_back("");

    bool is_dir = false;
    if (splitted_path_regex[splitted_path_regex.size() - 1].empty())
    {
      is_dir = true;
    }

    std::vector<std::string> reverse_names;
    for (auto it = splitted_path_regex.rbegin(); it != splitted_path_regex.rend(); it++) 
    {
      if (it->empty()) { continue; }
      reverse_names.push_back(*it);
    }
    
    int args_ptr = reverse_names.size() - 1;
    while (args_ptr >= 0)
    {
      std::vector<std::string> tmp_path_name;
      for (std::string& i : expanded_path_name)
      {
        if (flag)
        {
          i += '/';
        }
        DIR* dir = opendir((path + i).c_str());
        if (dir != nullptr)
        {
          for (dirent* d = readdir(dir); d != nullptr; d = readdir(dir))
          {
            if (d->d_name[0] == '.') { continue; }
            if (args_ptr > 0)
            {
              Matcher m(d->d_name, reverse_names[args_ptr].c_str());
              if (d->d_type == DT_DIR && m.match())
              {
                tmp_path_name.push_back(i + d->d_name);
              }
            }
            else
            {
              Matcher m(d->d_name, reverse_names[args_ptr].c_str());
              if ((d->d_type == DT_DIR || (d->d_type == DT_REG && !is_dir)) && m.match())
              {
                tmp_path_name.push_back(i + d->d_name);
              }
            }
          }
        }
        closedir(dir);
      }
      expanded_path_name = tmp_path_name;
      args_ptr--;
      flag = true;
    }

    if (expanded_path_name.empty())
    {
      return {};
    }

    return expanded_path_name;
  }

  /**********************************************************************/

  /**********************************************************************
   * Commands execution and related functions
   **********************************************************************/

  /* Returns current working directory absolute pathname */
  static std::string get_curr_dir()
  {
    char dir_name_C[PATH_MAX];

    if (getcwd(dir_name_C, PATH_MAX) == nullptr)
    {
      std::cerr << strerror(errno);
    };

    return std::string(dir_name_C);
  }

  /* Returns home directory pathname */
  static std::string get_home_dir()
  {
    const char *home_dir_name_C;
    if ((home_dir_name_C = getenv("HOME")) == nullptr) {
      home_dir_name_C = getpwuid(getuid())->pw_dir;
    }

    return std::string(home_dir_name_C);
  }

  /* Prints current directory to given output stream */
  static ERR_CODE print_intro_line(std::ostream &os)
  {
    gid_t gid = getgid();
    errno = 0;
    std::string intro_line = get_curr_dir();

    if (errno != 0)
    {
      ADD_LOG_WITH_RETURN(FAILURE, 0);
    }

    if (gid == 0)
    {
      intro_line += "!";
    }
    else // if (gid != 0)
    {
      intro_line += "> ";
    }

    os << BOLDCYAN << intro_line << RESET;
    return SUCCESS;
  }

  /* Executes command in current class variable */
  ERR_CODE exec()
  {
    switch (cmd_type)
    {
      case CMD_CD: {
        if (command_name.size() == 1) { IS_SUCCESS_WITH_RETURN(exec_cd(get_home_dir()))}
        else if (command_name.size() == 2) { IS_SUCCESS_WITH_RETURN(exec_cd(command_name[1]))}
        else { return FAILURE; }
        break;
      }

      case CMD_PWD: {
        IS_SUCCESS_WITH_RETURN(exec_pwd())
        break;
      }

      case CMD_SET: {
        IS_SUCCESS_WITH_RETURN(exec_set())
        break;
      }

      case CMD_OUT:
      {
        IS_SUCCESS_WITH_RETURN(io_redirect())
        exec_bash_command(command_name);
        break;
      }

      case CMD_TIME:
      {
        print_err(std::cerr, ERR_WRONG_INPUT);
        break;
      }
    }

    return SUCCESS;
  }

  /* Obtains i\o redirection */
  ERR_CODE io_redirect()
  {
    int fd_in = -1,
        fd_out = -1;

    if (!output_file_name.empty())
    {
      fd_out = open(output_file_name.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IWRITE | S_IREAD);
      if (fd_out == -1)
      {
        print_err(std::cerr, ERR_FILE_OPEN);
        ADD_LOG_WITH_RETURN(ERR_FILE_OPEN, 3);
      }
      dup2(fd_out, STDOUT_FILENO);
    }

    if (!input_file_name.empty())
    {
      fd_in = open(input_file_name.c_str(), O_RDONLY);
      if (fd_in == -1)
      {
        print_err(std::cerr, ERR_FILE_OPEN);
        ADD_LOG_WITH_RETURN(ERR_FILE_OPEN, 3);
      }
      dup2(fd_in, STDIN_FILENO);
    }

    return SUCCESS;
  }

  /* Executes 'cd' - change directory command */
  static ERR_CODE exec_cd(const std::string& new_dir)
  {
    if (chdir(new_dir.c_str()) == -1)
    {
      print_err(std::cerr, ERR_FILE_DIR_EXIST);
      ADD_LOG_WITH_RETURN(ERR_FILE_DIR_EXIST, 0);
    }

    return SUCCESS;
  }

  /* Executes 'pwd' - prints name of current/working directory */
  static ERR_CODE exec_pwd()
  {
    errno = 0;
    std::string curr_dir;
    curr_dir = get_curr_dir();
    if (errno != 0)
    {
      kill(getpid(), SIGINT);
      ADD_LOG_WITH_RETURN(FAILURE, 3);
    }

    std::cout << curr_dir << std::endl;
    return SUCCESS;
  }

  /* Executes 'set' - shows all shell-variables and environment variables */
  static ERR_CODE exec_set()
  {
    for (int i = 0; environ[i] != nullptr; i++)
    {
      std::cout << environ[i] << std::endl;
    }

    return SUCCESS;
  }

  static void exec_bash_command(const std::vector<std::string> &command_name)
  {
    errno = 0;
    std::vector<char*> v;

    v.reserve(command_name.size());
    for (const auto &s : command_name)
    {
      v.push_back((char *)s.c_str());
    }
    v.push_back(nullptr);
    execvp(v[0], &v[0]);
    perror(v[0]);      // TODO: error message and new intro_line print sequence is not determined
    kill(getpid(), SIGKILL);
  }

  /**********************************************************************/



  /**********************************************************************
   *
   **********************************************************************/

  /**********************************************************************/
};

#endif //MICROSHA_COMMAND_H
