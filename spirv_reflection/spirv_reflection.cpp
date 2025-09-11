#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include "kujogfx.h"
using namespace kujogfx;
using namespace std;

vector<uint32_t> readFile(string filename)
{
    vector<uint32_t> file_vec;

    ifstream file(filename, ios::in | ios::binary | ios::ate);

    if (!file.is_open())
    {
	return file_vec;
    }

    streampos size = file.tellg();
    vector<uint8_t> temp_vec(size, 0);
    file.seekg(0, ios::beg);
    file.read((char*)temp_vec.data(), size);
    cout << filename << " succesfully loaded." << endl;
    file.close();

    if ((temp_vec.size() % 4) != 0)
    {
	int extra_bytes = (4 - (temp_vec.size() % 4));

	for (int i = 0; i < extra_bytes; i++)
	{
	    temp_vec.push_back(0);
	}
    }

    for (size_t i = 0; i < temp_vec.size(); i += 4)
    {
	uint8_t high = temp_vec.at(i);
	uint8_t mid1 = temp_vec.at(i + 1);
	uint8_t mid2 = temp_vec.at(i + 2);
	uint8_t low = temp_vec.at(i + 3);

	uint32_t val = (uint32_t(high) << 24) | (uint32_t(mid1) << 16) | (uint32_t(mid2) << 8) | uint32_t(low);

	file_vec.push_back(val);
    }

    return file_vec;
}

string typeToString(const SPIRType &type)
{
    stringstream type_str;

    switch (type.basetype)
    {
	case SPIRType::BaseType::Unknown: type_str << "unk"; break;
	case SPIRType::BaseType::Float:
	{
	    cout << "Float type" << endl;
	    cout << "Width: " << dec << int(type.width) << endl;
	    cout << "Vec size: " << dec << int(type.vecsize) << endl;
	    cout << "Columns: " << dec << int(type.columns) << endl;
	    cout << endl;
	    type_str << "float";

	    if (type.vecsize > 1)
	    {
		type_str << dec << int(type.vecsize);
	    }
	}
	break;
	default: type_str << "unrec"; break;
    }

    return type_str.str();
}

bool compileHLSL(vector<uint32_t> spv_code)
{
    CompilerHLSL compiler(spv_code);
    CompilerHLSL::Options options;
    options.shader_model = 40;
    compiler.set_hlsl_options(options);

    auto resources = compiler.get_shader_resources();

    int index = 0;

    for (auto resource : resources.stage_inputs)
    {
	if (compiler.has_decoration(resource.id, spv::DecorationLocation))
	{
	    cout << "Decoration for index of " << dec << int(index) << " found" << endl;
	}

	uint32_t slot = compiler.get_decoration(resource.id, spv::DecorationLocation);

	stringstream slot_str;
	slot_str << "TEXCOORD" << dec << int(slot);
	cout << "Semantic name for index of " << dec << int(index) << ": " << slot_str.str() << endl;

	index += 1;
    }

    cout << endl;

    string src = compiler.compile();

    if (src.empty())
    {
	cout << "Could not translate SPIR-V to HLSL!" << endl;
	return false;
    }

    cout << "Shader HLSL: " << endl;
    cout << src << endl;
    return true;
}

bool compileGLSL(vector<uint32_t> spv_code)
{
    CompilerGLSL compiler(spv_code);
    CompilerGLSL::Options glsl_options;
    glsl_options.version = 140;
    glsl_options.es = false;
    glsl_options.vulkan_semantics = false;
    glsl_options.enable_420pack_extension = false;
    compiler.set_common_options(glsl_options);

    auto resources = compiler.get_shader_resources();

    int index = 0;

    for (auto resource : resources.stage_inputs)
    {
	cout << "GLSL semantic name for index of " << dec << int(index) << ": " << resource.name << endl;

	index += 1;
    }

    cout << endl;

    string src = compiler.compile();

    if (src.empty())
    {
	cout << "Could not translate SPIR-V to GLSL!" << endl;
	return false;
    }

    cout << "Shader GLSL: " << endl;
    cout << src << endl;
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
	cout << "Usage: spirv-reflection <.spv file>" << endl;
	return 1;
    }

    auto spv_code = readFile(argv[1]);

    if (spv_code.empty())
    {
	cout << "Could not read SPIR-V code file!" << endl;
	return 1;
    }

    cout << "Size: " << dec << int(spv_code.size()) << endl;

    if (!compileGLSL(spv_code))
    {
	return 1;
    }

    return 0;
}