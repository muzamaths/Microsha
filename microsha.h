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

  /* Default class destructor */
  ~Microsha()
  =default;

  /* Program execution loop */
  ERR_CODE Run();
};

#endif //MICROSHA_MICROSHA_H
