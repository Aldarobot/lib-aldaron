/** \file
 * jl_en.h
 * 	Enumerations.
**/

// Return Values
enum {
	JL_RTN_SUCCESS, // 0
	JL_RTN_FAIL, // 1
	JL_RTN_IMPOSSIBLE, // 2
	JL_RTN_SUPER_IMPOSSIBLE, // 3
	JL_RTN_COMPLETE_IMPOSSIBLE, // 4
	JL_RTN_FAIL_IN_FAIL_EXIT, // 5
} JL_RTN;

//ERROR MESSAGES
typedef enum{
	JL_ERR_NERR, //NO ERROR
	JL_ERR_NONE, //Something requested is Non-existant
	JL_ERR_FIND, //Can not find the thing requested
	JL_ERR_NULL, //Something requested is empty/null
}jl_err_t;

/*typedef enum{
	_JL_IO_MINIMAL,	//JL-lib prints starting/started/stopping etc.
	_JL_IO_PROGRESS,//JL-lib prints image/audio loading
	_JL_IO_SIMPLE,	//JL-lib prints all landmarks
	_JL_IO_INTENSE,	//JL-lib prints all debug info
	_JL_IO_MAX,
}jl_io_tag_t;

//IO TAGS
#define JL_IO_MINIMAL _JL_IO_MINIMAL - _JL_IO_MAX
#define JL_IO_PROGRESS _JL_IO_PROGRESS - _JL_IO_MAX
#define JL_IO_SIMPLE _JL_IO_SIMPLE - _JL_IO_MAX
#define JL_IO_INTENSE _JL_IO_INTENSE - _JL_IO_MAX*/
