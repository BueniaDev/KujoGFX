#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <optional>
#include "kujogfx.h"
using namespace kujogfx;
using namespace std;

bool loadShader(string filename, string &code)
{
    ifstream stream(filename, ios::in);

    if (!stream.is_open())
    {
	cout << "Error loading shader" << endl;
	return false;
    }

    stringstream str;
    str << stream.rdbuf();
    code = str.str();
    stream.close();

    return true;
}

array<optional<vector<uint32_t>>, 2> translateSPIRV(string vertex, string fragment)
{
    KujoGFXShader shader(vertex, fragment);
    array<optional<vector<uint32_t>>, 2> spirv_code;

    if (!shader.translateSPIRV(false))
    {
	return spirv_code;
    }

    spirv_code[0] = shader.vert_spirv;
    spirv_code[1] = shader.frag_spirv;
    return spirv_code;
}

array<optional<string>, 2> translateHLSL(string vertex, string fragment)
{
    KujoGFXShader shader(vertex, fragment);
    array<optional<string>, 2> spirv_code;

    if (!shader.translateHLSL())
    {
	return spirv_code;
    }

    spirv_code[0] = shader.vertex_out_src;
    spirv_code[1] = shader.fragment_out_src;
    return spirv_code;
}

array<optional<string>, 2> translateGLSL(string vertex, string fragment)
{
    KujoGFXShader shader(vertex, fragment);
    array<optional<string>, 2> spirv_code;

    if (!shader.translateGLSL())
    {
	return spirv_code;
    }

    spirv_code[0] = shader.vertex_out_src;
    spirv_code[1] = shader.fragment_out_src;
    return spirv_code;
}

bool disassembleSPIRV(vector<uint32_t> code, ostream &stream)
{
    size_t dasm_pointer = 0;
    bool is_end_of_code = false;

    auto getWord = [&]() -> uint32_t
    {
	if (dasm_pointer >= code.size())
	{
	    is_end_of_code = true;
	    return 0;
	}

	return code.at(dasm_pointer++);
    };

    auto skipWords = [&](size_t num_words) -> void
    {
	if (dasm_pointer >= code.size())
	{
	    return;
	}

	size_t num_words_skipped = num_words;

	if ((dasm_pointer + num_words) >= code.size())
	{
	    num_words_skipped = (code.size() - dasm_pointer);
	}

	dasm_pointer += num_words_skipped;
    };

    // Magic number
    uint32_t magic_number = getWord();

    if (magic_number != 0x7230203)
    {
	cout << "Invalid SPIR-V code" << endl;
	return false;
    }

    // Version number (only version 1.0 is supported ATM)
    uint32_t version_number = getWord();

    if (version_number != 0x00010000)
    {
	cout << "Invalid version number of " << hex << int(version_number) << endl;
	return false;
    }

    skipWords(3);

    while (!is_end_of_code)
    {
	uint32_t op_word = getWord();
	uint16_t opcode = uint16_t(op_word);
	uint16_t word_count = uint16_t(op_word >> 16);

	switch (opcode)
	{
	    default:
	    {
		stream << "unk (" << dec << int(opcode) << ", " << dec << int(word_count) << ")" << endl;
		// cout << "Unrecognized opcode of " << dec << int(opcode) << ", word count of " << dec << int(word_count) << endl;
		// return false;

		if (word_count > 0)
		{
		    skipWords(word_count - 1);
		}
	    }
	    break;
	}
    }

    return true;
}

bool spirvLogic(string vert, string frag)
{

    auto spirv_code = translateSPIRV(vert, frag);

    if (!spirv_code[0] || !spirv_code[1])
    {
	cout << "Could not compile shaders!" << endl;
	return false;
    }

    vector<uint32_t> vertex_code = spirv_code[0].value();
    vector<uint32_t> fragment_code = spirv_code[1].value();

    cout << "Vertex shader code size: " << dec << vertex_code.size() << endl;
    cout << "Fragment shader code size: " << dec << fragment_code.size() << endl;

    stringstream vert_stream;
    stringstream frag_stream;

    if (!disassembleSPIRV(vertex_code, vert_stream))
    {
	cout << "Could not disassemble vertex shader code!" << endl;
	return false;
    }

    cout << "Vertex shader disassembly: " << endl;
    cout << vert_stream.str() << endl;
    cout << endl;

    if (!disassembleSPIRV(fragment_code, frag_stream))
    {
	cout << "Could not disassemble vertex shader code!" << endl;
	return false;
    }

    cout << "Fragment shader disassembly: " << endl;
    cout << frag_stream.str() << endl;
    cout << endl;
    return true;
}

bool hlslLogic(string vert, string frag)
{
    auto hlsl_src = translateHLSL(vert, frag);


    if (!hlsl_src[0] || !hlsl_src[1])
    {
	cout << "Could not compile shaders!" << endl;
	return false;
    }

    cout << "Vertex shader output: " << endl;
    cout << hlsl_src[0].value() << endl;
    cout << endl;

    cout << "Fragment shader output: " << endl;
    cout << hlsl_src[1].value() << endl;
    cout << endl;

    return true;
}

bool glslLogic(string vert, string frag)
{
    auto glsl_src = translateGLSL(vert, frag);


    if (!glsl_src[0] || !glsl_src[1])
    {
	cout << "Could not compile shaders!" << endl;
	return false;
    }

    cout << "Vertex shader output: " << endl;
    cout << glsl_src[0].value() << endl;
    cout << endl;

    cout << "Fragment shader output: " << endl;
    cout << glsl_src[1].value() << endl;
    cout << endl;

    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
	cout << "Usage: spirv_test <vertex shader file> <fragment shader file>" << endl;
	return 1;
    }

    string vert_source = "";
    string frag_source = "";

    if (!loadShader(argv[1], vert_source))
    {
	cout << "Could not load vertex shader file!" << endl;
	return 1;
    }

    if (!loadShader(argv[2], frag_source))
    {
	cout << "Could not load fragment shader file!" << endl;
	return 1;
    }

    if (!hlslLogic(vert_source, frag_source))
    {
	return 1;
    }

    return 0;
}