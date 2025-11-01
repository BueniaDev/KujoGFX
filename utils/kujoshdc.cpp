// KujoSHDC - Official cross-shader translator for KujoGFX
// (Requires glslang, SPIRV-Cross and SPIRV-tools as dependencies)
// Compile:
// g++ kujoshdc.cpp --std=c++17 -O3 -lSPIRV -lSPIRV-tools -lglslang -lMachineIndependent -lOSDependent -lpthread -lGenericCodeGen -lspirv-cross-core -lspirv-cross-glsl -lspirv-cross-hlsl -o kujoshdc

#if defined(_WIN32) || defined(_WIN64)
#define KUJOSDHC_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__linux)
#define KUJOSDHC_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#define KUJOSDHC_PLATFORM_MACOS
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define KUJOSDHC_PLATFORM_BSD
#endif

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <spirv-tools/libspirv.h>
#include "spirv_cross/spirv_glsl.hpp"
#include "spirv_cross/spirv_hlsl.hpp"
using namespace spirv_cross;
using namespace std;

#include "shader_logic.inl"

struct ShaderCode
{
    string glsl_code = "";
    string glsl_es_code = "";
    string hlsl_5_0_code = "";
    string hlsl_4_0_code = "";
    vector<uint32_t> spv_code;
};

struct ShaderLocations
{
    vector<string> glsl_names;
    vector<pair<string, uint32_t>> hlsl_semantics;
    vector<uint32_t> spirv_locations;
};

bool translateCode(EShLanguage shader_type, string code, ShaderCode &out_code)
{
    if (!toSPIRV(shader_type, code, out_code.spv_code))
    {
	return false;
    }

    if (!toGLSL(out_code.spv_code, GLSL330, out_code.glsl_code))
    {
	return false;
    }

    if (!toGLSL(out_code.spv_code, GLSL300ES, out_code.glsl_es_code))
    {
	return false;
    }

    if (!toHLSL(out_code.spv_code, true, out_code.hlsl_5_0_code))
    {
	return false;
    }

    if (!toHLSL(out_code.spv_code, false, out_code.hlsl_4_0_code))
    {
	return false;
    }

    return true;
}

bool fetchLocations(string code, ShaderLocations &locations)
{
    vector<uint32_t> spv_code;

    if (!toSPIRV(EShLangVertex, code, spv_code))
    {
	return false;
    }

    locations.glsl_names = fetchNamesGLSL(spv_code);
    locations.hlsl_semantics = fetchSemanticsHLSL(spv_code);
    locations.spirv_locations = fetchLocationsSPIRV(spv_code);
    return true;
}

string printStringLiteral(string str)
{
    stringstream out_str;
    out_str << "/*" << endl;
    out_str << str;
    out_str << "*/" << endl;

    out_str << "{";

    for (size_t i = 0; i < str.size(); i++)
    {
	if ((i % 8) == 0)
	{
	    out_str << endl;
	    out_str << "    ";
	}

	out_str << "0x" << hex << setfill('0') << setw(2) << int(str.at(i));

	if (i != (str.size() - 1))
	{
	    out_str << ", ";
	}
    }

    out_str << "\n}";
    return out_str.str();
}

string locationsToString(ShaderLocations locations, string name)
{
    stringstream out_code;
    out_code << "KujoGFXShaderLocations " << name << " = {" << endl;

    out_code << "    {";

    for (size_t i = 0; i < locations.glsl_names.size(); i++)
    {
	out_code << "\"" <<  locations.glsl_names.at(i) << "\"";

	if (i != (locations.glsl_names.size() - 1))
	{
	    out_code << ", ";
	}

	if (((i + 1) % 8) == 0)
	{
	    out_code << endl;
	}
    }

    out_code << "}," << endl;
    out_code << "    {";

    for (size_t i = 0; i < locations.hlsl_semantics.size(); i++)
    {
	auto semantic = locations.hlsl_semantics.at(i);

	out_code << "{" << "\"" << semantic.first << "\", " << dec << semantic.second << "}";

	if (i != (locations.glsl_names.size() - 1))
	{
	    out_code << ", ";
	}

	if (((i + 1) % 8) == 0)
	{
	    out_code << endl;
	}
    }

    out_code << "}," << endl;
    out_code << "    {";

    for (size_t i = 0; i < locations.spirv_locations.size(); i++)
    {
	uint32_t loc_val = locations.spirv_locations.at(i);

	out_code << dec << loc_val;

	if (i != (locations.spirv_locations.size() - 1))
	{
	    out_code << ", ";
	}

	if (((i + 1) % 8) == 0)
	{
	    out_code << endl;
	}
    }

    out_code << "}" << endl;
    out_code << "};" << endl;

    return out_code.str();
}

string codeToString(ShaderCode code, string name)
{
    stringstream out_code;

    out_code << "KujoGFXShaderCodeDesc " << name << " = {" << endl;

    out_code << printStringLiteral(code.glsl_code) << "," << endl;
    out_code << printStringLiteral(code.glsl_es_code) << "," << endl;
    out_code << printStringLiteral(code.hlsl_5_0_code) << "," << endl;
    out_code << printStringLiteral(code.hlsl_4_0_code) << "," << endl;

    out_code << "{";

    for (size_t i = 0; i < code.spv_code.size(); i++)
    {
	if ((i % 8) == 0)
	{
	    out_code << endl;
	    out_code << "    ";
	}

	uint32_t code_val = code.spv_code.at(i);

	out_code << "0x" << hex << setfill('0') << setw(8) << code_val;

	if (i != (code.spv_code.size() - 1))
	{
	    out_code << ", ";
	}
    }

    out_code << "\n}" << endl;

    out_code << "};" << endl;

    return out_code.str();
}

void printUsage()
{
    cout << "Usage: kujoshdc <vertex shader> <fragment shader> <output>" << endl;
}

string loadFile(string filename)
{
    ifstream file(filename, ios::in);

    if (!file.is_open())
    {
	cout << "Could not open file of " << filename << endl;
	return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
	printUsage();
	return 1;
    }

    string vertex_src = loadFile(argv[1]);
    string fragment_src = loadFile(argv[2]);

    stringstream out_filename;
    out_filename << argv[3] << "_shader.inl";

    stringstream out_vertex;
    out_vertex << argv[3] << "_vertex";

    stringstream out_fragment;
    out_fragment << argv[3] << "_fragment";

    stringstream out_locations;
    out_locations << argv[3] << "_locations";

    ShaderCode vert_code;
    ShaderCode frag_code;
    ShaderLocations locations;

    if (!translateCode(EShLangVertex, vertex_src, vert_code))
    {
	cout << "Could not translate vertex shader to SPIR-V!" << endl;
	return 1;
    }

    if (!translateCode(EShLangFragment, fragment_src, frag_code))
    {
	cout << "Could not translate fragment shader to SPIR-V!" << endl;
	return 1;
    }

    if (!fetchLocations(vertex_src, locations))
    {
	cout << "Could not fetch vertex shader locations!" << endl;
	return 1;
    }

    ofstream out_file(out_filename.str(), ios::out);

    out_file << codeToString(vert_code, out_vertex.str()) << endl;

    out_file << codeToString(frag_code, out_fragment.str()) << endl;

    out_file << locationsToString(locations, out_locations.str()) << endl;

    out_file.close();

    return 0;
}