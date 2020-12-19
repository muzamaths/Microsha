#ifndef ONEGIN_ERROR_FUNCITONS_H
#define ONEGIN_ERROR_FUNCTIONS_H

/***
 * Note : do not use "errName" as ERRCODE type variable name, as it is used in macros
 */

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cassert>

#define LOG_FILE_NAME "log_out.txt"

/***
 * Adds log with the type of error, name of file, and line where it happened
 *
 * @param err_code - error code from enum ERR_CODE
 * @param offset - shows the offset upward from the line where macro was called
 * 					   to   the line where the error occurred
 */
#define ADD_LOG(err_type, offset)\
	add_log(LOG_FILE_NAME, err_type, __FILE__, __LINE__ - offset)

/***
 * Adds log with the type of error, name of file, and line where it happened
 * and finishes function with the return value 'err_type'
 *
 * @param err_code - error code from enum ERR_CODE
 * @param offset - shows the offset upward from the line where macro was called
 * 					   to   the line where the error occurred
 */
#define ADD_LOG_WITH_RETURN(err_type, offset)\
        add_log(LOG_FILE_NAME, err_type, __FILE__, __LINE__ - offset);\
        return err_type

/***
 * Compares variable 'var' with NULL-value. If the result is 'TRUE'
 * - adds log of the type of error, file, and line where the error occurred
 * - finishes the function with the return value 'err_type'
 *
 * @param - comparable variable
 */
#define NULL_CHECK_WITH_ERR_RETURN(var)						\
	if (var == NULL)							\
	{									\
	  add_log(LOG_FILE_NAME, ERR_NULL_PARAM, __FILE__, __LINE__ - 0);	\
	  return ERR_NULL_PARAM;						\
        }

/***
 * Checks if return value of function is 'SUCCESS'. If not, than this value is used as return value of function
 */
#define IS_SUCCESS_WITH_RETURN(ret_val)       \
        ERR_CODE errName = ret_val;           \
        if (errName != SUCCESS)               \
        {                                     \
          ADD_LOG_WITH_RETURN(errName, 0);    \
        }

/***
 * Error codes
 */
enum ERR_CODE
{
  SUCCESS = 0,        // successful finish of function.
  		      // Can be interpreted as success flag 'true' value
  FAILURE,	      // function caused no errors, but did not manage to do its purpose.
  		      // Can be interpreted as success flag 'false' value
  ERR_FILE_OPEN,      // error while opening file
  ERR_FILE_OPERATE,   // error while working with file
  ERR_UNEXP_EOF,      // unexpected end of file while reading file
  ERR_ALLOC,          // memory allocation error
  ERR_EXCESS_ALLOC,   // trying to allocate memory on already potentially allocated place
  ERR_NULL_PARAM,     // NULL-pointer is given as param to function
  ERR_NULL_ATTRIB,    // class contains NULL-attrib
  ERR_STAT,           // 'stat()' function failure error
  ERR_FUNC_IMPL,      // function implementation error
  ERR_OVERFLOW,	      // array overflow error
  ERR_UNDERFLOW,      // array overflow error
  ERR_FRONT_CANARY,   // structure front canary defect
  ERR_BACK_CANARY,    // structure back canary defect
  ERR_HASH_BREAK,     // hash function value defect
  ERR_WRONG_INPUT,    // wrong input format
  ERR_FILE_DIR_EXIST  // file or directory does not exist
};

/***
 * Creates new log text file
 *
 * @param const char * log_file_name - log file name
 *
 * @return error code. It can occur because of problems with creating/opening log file.
 */
enum ERR_CODE start_log( const char * log_file_name );

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
enum ERR_CODE add_log( const char * log_file_name, enum ERR_CODE code, const char * source_file_name, int curr_line );

/***
 * Prints error description by enum value
 *
 * @param std::ostream &os - reference to the output stream
 * @param const ERR_CODE &code - error code to print
 */
void print_err(std::ostream &os, const ERR_CODE &code);

#endif //ONEGIN_ERROR_FUNCITONS_H
