#pragma once

/**
 * Platform-independent part of graphics library
 */

#ifdef DEBUG
void __printShaderCompilationErrors(GLuint id);
void __printProgramLinkingErrors(GLuint id);
void __checkError();
#endif

/* Constants */
const float __NULLVEC[2] = {0,0};
const float __NULLMAT[4] = {0,0,0,0};
const float __UNIMAT[4] = {1,0,0,1};

struct __Buffers
{
	GLuint quad_buffer;
};

struct __VertexShaderUniforms
{
	GLint trans;
	GLint proj;
	GLint view;
};

struct __VertexShaderAttribs
{
	GLint coord;
};

const GLchar *__vertex_shader_source =
		"uniform vec2 uTranslation;\n"
		"uniform mat2 uProjection;\n"
		"uniform mat2 uModelview;\n"
		"attribute vec2 aCoord;\n"
    "varying vec2 vPosition;\n"
		"void main(void) {\n"
    "\tvPosition = aCoord;\n"
		"\tgl_Position = vec4(uProjection*uModelview*(aCoord + uTranslation),0.0,1.0);\n"
		"}\n";

struct __FragmentFillShaderUniforms
{
	GLint color;
};

const GLchar *__fragment_fill_shader_source =
		"uniform vec4 uColor;\n"
    "varying vec2 vPosition;\n"
		"void main(void) {\n"
		"\tgl_FragColor = uColor;\n"
		"}\n";

struct __FragmentCircleShaderUniforms
{
	GLint color;
	GLint center;
	GLint radius;
};

const GLchar *__fragment_circle_shader_source =
		"uniform vec4 uColor;\n"
    "varying vec2 vPosition;\n"
		"void main(void) {\n"
		"\tgl_FragColor = uColor*float(length(vPosition) <= 1.0);\n"
		"}\n";

struct __FillProgram
{
	GLuint id;
	struct __VertexShaderUniforms vs_uniforms;
	struct __VertexShaderAttribs vs_attribs;
	struct __FragmentFillShaderUniforms fs_uniforms;
};

struct __CircleProgram
{
	GLuint id;
	struct __VertexShaderUniforms vs_uniforms;
	struct __VertexShaderAttribs vs_attribs;
	struct __FragmentCircleShaderUniforms fs_uniforms;
};

struct __ProgramSet
{
	GLuint vert;
	GLuint frag_fill;
	GLuint frag_circle;
	struct __FillProgram prog_fill;
	struct __CircleProgram prog_circle;
};

struct __CommonContext
{
	int width, height;
	struct __ProgramSet shaders;
	struct __Buffers buffers;
	unsigned int fill_color;
	unsigned int stroke_color;
	float projection_matrix[4];
	float modelview_matrix[4];
	float translation[2];
	float color[4];
}
__context;

GLint __pullUniform(GLuint prog, const char *name)
{
	GLint unif;
	unif = glGetUniformLocation(prog, name);
#ifdef DEBUG
	if(unif == -1)
	{
		printf("Could not bind uniform '%s'\n",name);
	}
#endif
	return unif;
}

GLint __pullAttribute(GLuint prog, const char *name)
{
	GLint attr;
	attr = glGetAttribLocation(prog, name);
#ifdef DEBUG
	if(attr == -1)
	{
		printf("Could not bind attrib '%s'\n",name);
	}
#endif
	return attr;
}

void __loadShaders(struct __ProgramSet *set)
{
	set->vert = glCreateShader(GL_VERTEX_SHADER);
	set->frag_fill = glCreateShader(GL_FRAGMENT_SHADER);
	set->frag_circle = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(set->vert, 1, &(__vertex_shader_source), NULL);
	glShaderSource(set->frag_fill, 1, &(__fragment_fill_shader_source), NULL);
	glShaderSource(set->frag_circle, 1, &(__fragment_circle_shader_source), NULL);

	glCompileShader(set->vert);
	glCompileShader(set->frag_fill);
	glCompileShader(set->frag_circle);

#ifdef DEBUG
	__printShaderCompilationErrors(set->vert);
	__printShaderCompilationErrors(set->frag_fill);
	__printShaderCompilationErrors(set->frag_circle);
#endif

	set->prog_fill.id = glCreateProgram();
	set->prog_circle.id = glCreateProgram();

	glAttachShader(set->prog_fill.id, set->vert);
	glAttachShader(set->prog_fill.id, set->frag_fill);
	
	glAttachShader(set->prog_circle.id, set->vert);
	glAttachShader(set->prog_circle.id, set->frag_circle);

	glLinkProgram(set->prog_fill.id);
	glLinkProgram(set->prog_circle.id);

#ifdef DEBUG
	__printProgramLinkingErrors(set->prog_fill.id);
	__printProgramLinkingErrors(set->prog_circle.id);
#endif

	set->prog_fill.vs_uniforms.trans = __pullUniform(set->prog_fill.id, "uTranslation");
	set->prog_fill.vs_uniforms.proj = __pullUniform(set->prog_fill.id, "uProjection");
	set->prog_fill.vs_uniforms.view = __pullUniform(set->prog_fill.id, "uModelview");
	set->prog_fill.vs_attribs.coord = __pullAttribute(set->prog_fill.id, "aCoord");
	set->prog_fill.fs_uniforms.color = __pullUniform(set->prog_fill.id, "uColor");
	
	set->prog_circle.vs_uniforms.trans = __pullUniform(set->prog_circle.id, "uTranslation");
	set->prog_circle.vs_uniforms.proj = __pullUniform(set->prog_circle.id, "uProjection");
	set->prog_circle.vs_uniforms.view = __pullUniform(set->prog_circle.id, "uModelview");
	set->prog_circle.vs_attribs.coord = __pullAttribute(set->prog_circle.id, "aCoord");
	set->prog_circle.fs_uniforms.color = __pullUniform(set->prog_circle.id, "uColor");
}

void __deleteShaders(struct __ProgramSet *set)
{
	glDetachShader(set->prog_fill.id, set->vert);
	glDetachShader(set->prog_fill.id, set->frag_fill);
	
	glDetachShader(set->prog_circle.id, set->vert);
	glDetachShader(set->prog_circle.id, set->frag_circle);

	glDeleteProgram(set->prog_fill.id);
	glDeleteProgram(set->prog_circle.id);

	glDeleteShader(set->vert);
	glDeleteShader(set->frag_fill);
	glDeleteShader(set->frag_circle);
}

void __loadBuffers(struct __Buffers *buffers)
{
	glGenBuffers(1, &buffers->quad_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffers->quad_buffer);
	float quad[8] = {
		1.0f,1.0f,
		-1.0f,1.0f,
		-1.0f,-1.0f,
		1.0f,-1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(float), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void __deleteBuffers(struct __Buffers *buffers)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buffers->quad_buffer);
}


void translate(const float *vector)
{
	__context.translation[0] = vector[0];
	__context.translation[1] = vector[1];
}

void transform(const float *matrix)
{
	__context.modelview_matrix[0] = matrix[0];
	__context.modelview_matrix[1] = matrix[1];
	__context.modelview_matrix[2] = matrix[2];
	__context.modelview_matrix[3] = matrix[3];
}

#define __red(c)   ((c>>0)&0xff)
#define __green(c) ((c>>8)&0xff)
#define __blue(c)  ((c>>16)&0xff)
#define __alpha(c) ((c>>24)&0xff)

#define __tofloat(c) (c*(1.0f/255.0f))

void setColor(unsigned color)
{
	__context.color[0] = __tofloat(__red(color));
	__context.color[1] = __tofloat(__green(color));
	__context.color[2] = __tofloat(__blue(color));
	__context.color[3] = __tofloat(__alpha(color));
}

void setFloatColor(const float *color)
{
	__context.color[0] = color[0];
	__context.color[1] = color[1];
	__context.color[2] = color[2];
	__context.color[3] = color[3];
}

void clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void __setModelviewUniforms(struct __VertexShaderUniforms *unifs, const float *v, const float *m, const float *p)
{
	glUniform2fv(unifs->trans, 1, v);
	glUniformMatrix2fv(unifs->proj, 1, GL_FALSE, p);
	glUniformMatrix2fv(unifs->view, 1, GL_FALSE, m);
}

void __setFillColorUniform(const float *c)
{
	glUniform4fv(__context.shaders.prog_fill.fs_uniforms.color, 1, c);
}

void __setCircleColorUniform(const float *c)
{
	glUniform4fv(__context.shaders.prog_circle.fs_uniforms.color, 1, c);
}

void __useFillProgram(const float *vec, const float *mv, const float *proj, const float *col)
{
	glUseProgram(__context.shaders.prog_fill.id);
	{
		__setModelviewUniforms(&__context.shaders.prog_fill.vs_uniforms,vec,mv,proj);
		__setFillColorUniform(col);
		
		glEnableVertexAttribArray(__context.shaders.prog_fill.vs_attribs.coord);
		{
			glBindBuffer(GL_ARRAY_BUFFER, __context.buffers.quad_buffer);
			glVertexAttribPointer(__context.shaders.prog_fill.vs_attribs.coord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_QUADS,0,4);
		}
		glDisableVertexAttribArray(__context.shaders.prog_fill.vs_attribs.coord);
	}
	glUseProgram(0);
}

void __useCircleProgram(const float *vec, const float *mv, const float *proj, const float *col)
{
	glUseProgram(__context.shaders.prog_circle.id);
	{
		__setModelviewUniforms(&__context.shaders.prog_circle.vs_uniforms,vec,mv,proj);
		__setCircleColorUniform(col);
		
		glEnableVertexAttribArray(__context.shaders.prog_circle.vs_attribs.coord);
		{
			glBindBuffer(GL_ARRAY_BUFFER, __context.buffers.quad_buffer);
			glVertexAttribPointer(__context.shaders.prog_circle.vs_attribs.coord, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_QUADS,0,4);
		}
		glDisableVertexAttribArray(__context.shaders.prog_circle.vs_attribs.coord);
	}
	glUseProgram(0);
}

void fill()
{
	__useFillProgram(__NULLVEC,__UNIMAT,__UNIMAT,__context.color);
}

void quad()
{
	__useFillProgram(__context.translation,__context.modelview_matrix,__context.projection_matrix,__context.color);
}

void circle()
{
	__useCircleProgram(__context.translation,__context.modelview_matrix,__context.projection_matrix,__context.color);
}
