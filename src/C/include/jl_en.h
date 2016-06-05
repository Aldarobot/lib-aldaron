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

typedef enum{
	JL_GL_SLPR_TEX, //Texture Shader
	JL_GL_SLPR_PRM, //Pre-Blended Texture Shader
	JL_GL_SLPR_CLR, //Color Shader
	JL_GL_SLPR_CHR, //Character Shader
	JL_GL_SLPR_MAX,
}jl_gl_slpr;

//Image ID's
typedef enum{
	JL_IMGI_LOAD,
	JL_IMGI_FONT,
	JL_IMGI_ICON,
	JL_IMGI_GAME,
}jl_imgi_t;

typedef enum{
	JL_IMG_FORMAT_IMG=1,
	JL_IMG_FORMAT_HQB=2,
	JL_IMG_FORMAT_PIC=3,
	JL_IMG_FORMAT_FLS=4,
}jl_img_format_t;

typedef enum{
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
#define JL_IO_INTENSE _JL_IO_INTENSE - _JL_IO_MAX
