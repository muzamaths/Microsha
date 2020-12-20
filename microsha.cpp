#include "microsha.h"

int signal_value;

/* SIGINT signal handler */
static void SIGINT_handler(int sig_value)
{
  signal_value = sig_value;
}

/* Program execution loop */
ERR_CODE Microsha::Run()
{
  command_pipeline pipeline{};
  std::string command_line{};
  signal(SIGINT, SIGINT_handler);

  while (true)
  {
    signal_value = 0;
    command::print_intro_line(std::cout);

    //TODO: something is wrong here. Signal : sighup is thrown. But if 'break' is removed lool becomes infinite
    if (!getline(std::cin, command_line))
    {
      std::cout << std::endl;
      break;
    }

    if (signal_value == SIGINT) {
      continue;
    }

    pipeline.reset_pipeline(command_line);
    pipeline.exec();

  }

  return SUCCESS;
}
