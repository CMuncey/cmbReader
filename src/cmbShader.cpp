#include <string>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "cmbShader.hpp"
#include "cmb.h"
#include "matsChunk.h"
using namespace std;

string cmbShader_t::makeTCSrc(uint16_t src)
{
	switch(src)
	{
		case 0x84C0:
			return("texture(tex0, fin.tex0Coord)");
		case 0x84C1:
			return("texture(tex1, fin.tex1Coord)");
		case 0x84C2:
			return("texture(tex2, fin.tex2Coord)");
		case 0x84C3:
			return("vec4(1.0f)");
		case 0x8576:
			return("cCol");
		case 0x8578:
			return("outCol");
		case 0x8579:
			return("bufCol");
		case 0x6210:
		case 0x6211:
		case 0x8577:
		default:
			return("(fin.color)");
	}
}

string cmbShader_t::makeTCOp(uint16_t src, uint16_t op)
{
	switch(op)
	{
		case 0x0301:
			return("(1.0f - " + makeTCSrc(src) + ".rgba)");
		case 0x0302:
			return("(" + makeTCSrc(src) + ".aaaa)");
		case 0x0303:
			return("(1.0f - " + makeTCSrc(src) + ".aaaa)");
		case 0x8580:
			return("(" + makeTCSrc(src) + ".rrrr)");
		case 0x8581:
			return("(" + makeTCSrc(src) + ".gggg)");
		case 0x8582:
			return("(" + makeTCSrc(src) + ".bbbb)");
		case 0x8583:
			return("(1.0f - " + makeTCSrc(src) + ".rrrr)");
		case 0x8584:
			return("(1.0f - " + makeTCSrc(src) + ".gggg)");
		case 0x8585:
			return("(1.0f - " + makeTCSrc(src) + ".bbbb)");
		case 0x0300:
		default:
			return(makeTCSrc(src));
	}
}

string cmbShader_t::makeTCCombine(uint16_t cmb)
{
	switch(cmb)
	{
		case 0x0104:
			return("(inCol0 + inCol1)");
		case 0x2100:
			return("(inCol0 * inCol1)");
		case 0x6401:
			return("((inCol0 * inCol1) + inCol2)");
		case 0x6402:
			return("((inCol0 + inCol1) * inCol2)");
		case 0x84E7:
			return("(inCol0 - inCol1)");
		case 0x8574:
			return("((inCol0 + inCol1) - 0.5f)");
		case 0x8575:
			return("(mix(inCol0, inCol1, inCol2))");
		case 0x86AE:
			return("(vec4(vec3(4.0f * (dot(inCol0 - 0.5f, inCol1 - 0.5f))), 1.0f))");
		case 0x86AF:
			return("(vec4(4.0f * (dot(inCol0 - 0.5f, inCol1 - 0.5f))))");
		case 0x1E01:
		default:
			return("inCol0");
	}
}

string cmbShader_t::makeTCScale(uint16_t cmb, uint16_t scl)
{
	switch(scl)
	{
		case 4:
			return("(" + makeTCCombine(cmb) + " * 4.0f)");
		case 2:
			return("(" + makeTCCombine(cmb) +  " * 2.0f)");
		case 1:
		default:
			return(makeTCCombine(cmb));
	}
}

string cmbShader_t::makeTCBuf(uint16_t buf)
{
	switch(buf)
	{
		case 0x8578:
			return("outCol");
		case 0x8579:
		default:
			return("bufCol");
	}
}

string cmbShader_t::makeTC(texCombiner_t* tc)
{
	string s;

	s  = "\tcCol = constCol[" + to_string(tc->constInd) + "];\n";

	s += "\tinCol0 = vec4(";
	s += makeTCOp(tc->srcRGB[0], tc->opRGB[0]) + ".rgb, ";
	s += makeTCOp(tc->srcAlpha[0], tc->opAlpha[0]) + ".a);\n";

	s += "\tinCol1 = vec4(";
	s += makeTCOp(tc->srcRGB[1], tc->opRGB[1]) + ".rgb, ";
	s += makeTCOp(tc->srcAlpha[1], tc->opAlpha[1]) + ".a);\n";

	s += "\tinCol2 = vec4(";
	s += makeTCOp(tc->srcRGB[2], tc->opRGB[2]) + ".rgb, ";
	s += makeTCOp(tc->srcAlpha[2], tc->opAlpha[2]) + ".a);\n";

	s += "\toutCol = vec4(";
	s += makeTCScale(tc->combineRGB, tc->scaleRGB) + ".rgb, ";
	s += makeTCScale(tc->combineAlpha, tc->scaleAlpha) + ".a);\n";

	s += "\tbufCol = vec4(";
	s += makeTCBuf(tc->bufInpRGB) + ".rgb, ";
	s += makeTCBuf(tc->bufInpRGB) + ".a);\n";

	return(s);
}

string cmbShader_t::makeAlphaTest(uint16_t func, uint8_t val)
{
	float fVal;

	fVal = val / 255.0f;

	switch(func)
	{
		case 0x0200:
			return("true");
		case 0x0201:
			return("outCol.a >= " + to_string(fVal));
		case 0x0202:
			return("outCol.a != " + to_string(fVal));
		case 0x0203:
			return("outCol.a > " + to_string(fVal));
		case 0x0204:
			return("outCol.a <= " + to_string(fVal));
		case 0x0205:
			return("outCol.a == " + to_string(fVal));
		case 0x0206:
			return("outCol.a < " + to_string(fVal));
		case 0x0207:
		default:
			return("false");
	}
}

string cmbShader_t::makeFS(const cmb_t* cmb, int matInd)
{
	material_t* mat;
	string s;
	uint32_t i;

	mat = &(cmb->matsC->mats[matInd]);

	s  = "vec4 fsFunc()\n{\n";
	s += "\tvec4 inCol0, inCol1, inCol2, outCol, bufCol, cCol;\n\n";

	s += "\tbufCol = clamp(vec4(";
	s += to_string(mat->bufColor[0]) + ", ";
	s += to_string(mat->bufColor[1]) + ", ";
	s += to_string(mat->bufColor[2]) + ", ";
	s += to_string(mat->bufColor[3]) + "), vec4(0.0f), vec4(1.0f));\n\n";

	for(i = 0; i < mat->nTexCombiners; ++i)
		s += makeTC(&(cmb->matsC->tCom[mat->texCInd[i]])) + "\n";

	s += "\tif(";
	s += makeAlphaTest(mat->alphaFunc, mat->alphaRefVal);
	s += ")\n\t\tdiscard;\n\n";

	s += "\treturn(outCol);\n}\n";

	return(s);
}

cmbShader_t::cmbShader_t(const cmb_t* cmb, int matInd)
{
	FILE* f;
	char* vCode;
	char* fCode;
	string fCode2;
	int tmp, size;
	char iLog[512];
	unsigned int vSh, fSh;

	/* Read the vertex shader */
	f = fopen("src/cmbVS.glsl", "r");
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	vCode = (char*)calloc(size + 1, sizeof(char));
	fread(vCode, 1, size, f);
	fclose(f);

	/* Read the start of the fragment shader */
	f = fopen("src/cmbFS.glsl", "r");
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fCode = (char*)calloc(size + 1, sizeof(char));
	fread(fCode, 1, size, f);
	fclose(f);
	fCode2 = fCode;
	free(fCode);

	/* Generate the rest of the fragment shader out of texture combiners */
	fCode2 += makeFS(cmb, matInd);

//if(matInd == 24)
//{
//printf("Shader for material %d\n", matInd);
//printf("========================================================================================\n");
//printf("%s\n", fCode2.c_str());
//printf("========================================================================================\n");
//}

	/* Make the vertex shader */
	vSh = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vSh, 1, &vCode, NULL);
	glCompileShader(vSh);
	glGetShaderiv(vSh, GL_COMPILE_STATUS, &tmp);
	if(!tmp)
	{
		glGetShaderInfoLog(vSh, 512, NULL, iLog);
		fprintf(stderr, "vShader compilation failed\n%s\n", iLog);
	}

	fCode = (char*)fCode2.c_str();
	fSh = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fSh, 1, &fCode, NULL);
	glCompileShader(fSh);
	glGetShaderiv(fSh, GL_COMPILE_STATUS, &tmp);
	if(!tmp)
	{
		glGetShaderInfoLog(fSh, 512, NULL, iLog);
		fprintf(stderr, "fShader compilation failed\n%s\n", iLog);
	}

	/* Link the shaders */
	id = glCreateProgram();
	glAttachShader(id, vSh);
	glAttachShader(id, fSh);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &tmp);
	if(!tmp)
	{
		glGetProgramInfoLog(id, 512, NULL, iLog);
		fprintf(stderr, "Linking failed\n%s\n", iLog);
	}

	glDeleteShader(vSh);
	glDeleteShader(fSh);
	free(vCode);
}

void cmbShader_t::use()
{
	glUseProgram(id);
}

void cmbShader_t::del()
{
	glDeleteShader(id);
}

unsigned int cmbShader_t::loc(const char* n) const
{
	return(glGetUniformLocation(id, n));
}

void cmbShader_t::set1i(const char* n, int v) const
{
	glUniform1i(glGetUniformLocation(id, n), v);
}

void cmbShader_t::set2i(const char* n, int v1, int v2) const
{
	glUniform2i(glGetUniformLocation(id, n), v1, v2);
}

void cmbShader_t::set2i(const char* n, const glm::ivec2 &v) const
{
	glUniform2iv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3i(const char* n, int v1, int v2, int v3) const
{
	glUniform3i(glGetUniformLocation(id, n), v1, v2, v3);
}

void cmbShader_t::set3i(const char* n, const glm::ivec3 &v) const
{
	glUniform3iv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set1f(const char* n, float v) const
{
	glUniform1f(glGetUniformLocation(id, n), v);
}

void cmbShader_t::set2f(const char* n, float v1, float v2) const
{
	glUniform2f(glGetUniformLocation(id, n), v1, v2);
}

void cmbShader_t::set2f(const char* n, const glm::vec2 &v) const
{
	glUniform2fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3f(const char* n, float v1, float v2, float v3) const
{
	glUniform3f(glGetUniformLocation(id, n), v1, v2, v3);
}

void cmbShader_t::set3f(const char* n, const glm::vec3 &v) const
{
	glUniform3fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set4f(const char* n, const glm::vec4 &v) const
{
	glUniform4fv(glGetUniformLocation(id, n), 1, &v[0]);
}

void cmbShader_t::set3mf(const char* n, const glm::mat3 &v) const
{
	unsigned int loc;

	loc = glGetUniformLocation(id, n);
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
}

void cmbShader_t::set4mf(const char* n, const glm::mat4 &v) const
{
	unsigned int loc;

	loc = glGetUniformLocation(id, n);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
}
