#ifndef MICROSHA_MICROSHA_H
#define MICROSHA_MICROSHA_H

#include "command_pipeline.h"

/* Micro shell program class declaration */
class Microsha
{
private:

public:
  /* Default class constructor */
  Microsha()
  =default;

  /* Program execution loop */
  ERR_CODE Run()
  {
    std::ostream &os = std::cout;
    command_pipeline pipeline;
    std::string command_line;

    while (true)
    {
      command::print_intro_line(os);
      if (!getline(std::cin, command_line)) //TODO: something is wrong here. Signal : sighup is thrown. But if 'break' is removed lool becomes infinite
      {
        break;
      }

      ERR_CODE error_code = pipeline.reset_pipeline(command_line);
      if (error_code != SUCCESS)
      {
        print_err(os, error_code);
        return error_code;
      }
    }

    return SUCCESS;
  }
};

#endif //MICROSHA_MICROSHA_H
