/// @ref core
/// @file glm/ext/matrix_double3x4.hpp

#pragma once
#include "../detail/type_mat3x4.hpp"

namespace glm
{
	/// @addtogroup core
	/// @{

#if(defined(GLM_PRECISION_LOWP_DOUBLE))
	typedef mat<3, 4, double, lowp>			dmat3x4;
#elif(defined(GLM_PRECISION_MEDIUMP_DOUBLE))
	typedef mat<3, 4, double, mediump>		dmat3x4;
#else //defined(GLM_PRECISION_HIGHP_DOUBLE)
	/// 3 columns of 4 components matrix of double-precision floating-point numbers.
	///
	/// @see <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.pdf">GLSL 4.20.8 specification, section 4.1.6 Matrices</a>
	typedef mat<3, 4, double, highp>		dmat3x4;
#endif

	/// @}
}//namespace glm
