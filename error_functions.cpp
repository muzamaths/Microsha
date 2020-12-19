#include "error_functions.h"

/***
 * Creates new log text file
 *
 * @param const char * log_file_name - log file name
 *
 * @return error code. It can occur because of problems with creating/opening log file.
 */
enum ERR_CODE start_log( const char * log_file_name )
{
  FILE *log_file;

  if((log_file = fopen(LOG_FILE_NAME, "w")) == nullptr)
  {
    printf("ERROR in '%s' file:\n \tSomething is wrong with creating/opening log file on line %i\n", __FILE__, __LINE__);
    return ERR_FILE_OPEN;
  }

  fprintf(log_file, "Logging start\n\n");

  fclose(log_file);

  return SUCCESS;
}

/***
 * Adds error information to log file
 *
 * @param const char * log_file_name - log file name
 * @param enum ERR_CODE code - code of error
 * @param const char * source_file_name - source code file name where mistake probably occurred
 * @param int curr_line - line of code, where error occurred
 *
 * @return error code. It can occur because of problems with writing/existence of log file.
 */
enum ERR_CODE add_log( const char * log_file_name, enum ERR_CODE code, const char * source_file_name, int curr_line )
{
  FILE *log_file;

  if((log_file = fopen(LOG_FILE_NAME, "w")) == nullptr)
  {
    printf("ERROR in '%s' file:\n \tSomething is wrong with opening log file on line %i\n", source_file_name, curr_line);
    return ERR_FILE_OPEN;
  }

  fprintf(log_file, "Error file : line - %s : %i\n\t", source_file_name, curr_line);

  switch(code)
  {
    case SUCCESS:
      fprintf(log_file, "Everything is OK\n\n");
      break;

    case FAILURE:
      fprintf(log_file, "Function failed to do its purpose\n\n");
      break;

    case ERR_FILE_OPEN:
      fprintf(log_file, "Could not open file\n\n");
      break;

    case ERR_FILE_OPERATE:
      fprintf(log_file, "Error while working with file\n\n");
      break;

    case ERR_UNEXP_EOF:
      fprintf(log_file, "Unexpected end of file while reading file\n\n");
      break;

    case ERR_ALLOC:
      fprintf(log_file, "Memory allocation error\n\n");
      break;

    case ERR_EXCESS_ALLOC:
      fprintf(log_file, "Trying to alloc memory by pointer which was already used for memory allocation\n\n");
      break;

    case ERR_NULL_PARAM:
      fprintf(log_file, "nullptr is given as function in param\n\n");
      break;

    case ERR_NULL_ATTRIB:
      fprintf(log_file, "class contains NULL attribute, but must not\n\n");
      break;

    case ERR_STAT:
      fprintf(log_file, "'stat()' function failure error\n\n");
      break;

    case ERR_FUNC_IMPL:
      fprintf(log_file, "function implementation error\n\n");
      break;

    case ERR_OVERFLOW:
      fprintf(log_file, "array overflow\n\n");
      break;

    case ERR_UNDERFLOW:
      fprintf(log_file, "array underflow\n\n");
      break;

    case ERR_FRONT_CANARY:
      fprintf(log_file, "structure front canary defect\n\n");
      break;

    case ERR_BACK_CANARY:
      fprintf(log_file, "structure back canary defect\n\n");
      break;

    case ERR_HASH_BREAK:
      fprintf(log_file, "hash function value defect\n\n");
      break;

    case ERR_WRONG_INPUT:
      fprintf(log_file, "Wrong input format\n\n");
      break;

    case ERR_FILE_DIR_EXIST:
      fprintf(log_file, "File or directory does not exist\n\n");
      break;


    default:
      fprintf(log_file, "Unknown error code\n\n");
      break;
  }

  fclose(log_file);

  return SUCCESS;
}


/***
 * Prints error description by enum value
 *
 * @param std::ostream &os - reference to the output stream
 * @param const ERR_CODE &code - error code to print
 */
void print_err(std::ostream &os, const ERR_CODE &code)
{
  switch(code)
  {
    case SUCCESS:
      os << "Everything is OK\n";
      break;

    case FAILURE:
      os << "Function failed to do its purpose\n";
      break;

    case ERR_FILE_OPEN:
      os << "Could not open file\n";
      break;

    case ERR_FILE_OPERATE:
      os << "Error while working with file\n";
      break;

    case ERR_UNEXP_EOF:
      os << "Unexpected end of file while reading file\n";
      break;

    case ERR_ALLOC:
      os << "Memory allocation error\n";
      break;

    case ERR_EXCESS_ALLOC:
      os << "Trying to alloc memory by pointer which was already used for memory allocation\n";
      break;

    case ERR_NULL_PARAM:
      os << "nullptr is given as function in param\n";
      break;

    case ERR_NULL_ATTRIB:
      os << "Class contains NULL attribute, but must not\n";
      break;

    case ERR_STAT:
      os << "'stat()' function failure error\n";
      break;

    case ERR_FUNC_IMPL:
      os << "Function implementation error\n";
      break;

    case ERR_OVERFLOW:
      os << "Array overflow\n";
      break;

    case ERR_UNDERFLOW:
      os << "Array underflow\n";
      break;

    case ERR_FRONT_CANARY:
      os << "Structure front canary defect\n";
      break;

    case ERR_BACK_CANARY:
      os << "Structure back canary defect\n";
      break;

    case ERR_HASH_BREAK:
      os << "Hash function value defect\n";
      break;

    case ERR_WRONG_INPUT:
      os << "Wrong input format\n";
      break;

    case ERR_FILE_DIR_EXIST:
      os << "File or directory does not exist\n";
      break;

    default:
      os << "Unknown error code\n";
      break;
  }
}