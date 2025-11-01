enum GLSLShaderLang : int
{
    GLSL330,
    GLSL300ES
};

void initResources(TBuiltInResource &resources)
{
    resources.maxLights = 32;
    resources.maxClipPlanes = 6;
    resources.maxTextureUnits = 32;
    resources.maxTextureCoords = 32;
    resources.maxVertexAttribs = 64;
    resources.maxVertexUniformComponents = 4096;
    resources.maxVaryingFloats = 64;
    resources.maxVertexTextureImageUnits = 32;
    resources.maxCombinedTextureImageUnits = 80;
    resources.maxTextureImageUnits = 32;
    resources.maxFragmentUniformComponents = 4096;
    resources.maxDrawBuffers = 32;
    resources.maxVertexUniformVectors = 128;
    resources.maxVaryingVectors = 8;
    resources.maxFragmentUniformVectors = 16;
    resources.maxVertexOutputVectors = 16;
    resources.maxFragmentInputVectors = 15;
    resources.minProgramTexelOffset = -8;
    resources.maxProgramTexelOffset = 7;
    resources.maxClipDistances = 8;
    resources.maxComputeWorkGroupCountX = 65535;
    resources.maxComputeWorkGroupCountY = 65535;
    resources.maxComputeWorkGroupCountZ = 65535;
    resources.maxComputeWorkGroupSizeX = 1024;
    resources.maxComputeWorkGroupSizeY = 1024;
    resources.maxComputeWorkGroupSizeZ = 64;
    resources.maxComputeUniformComponents = 1024;
    resources.maxComputeTextureImageUnits = 16;
    resources.maxComputeImageUniforms = 8;
    resources.maxComputeAtomicCounters = 8;
    resources.maxComputeAtomicCounterBuffers = 1;
    resources.maxVaryingComponents = 60;
    resources.maxVertexOutputComponents = 64;
    resources.maxGeometryInputComponents = 64;
    resources.maxGeometryOutputComponents = 128;
    resources.maxFragmentInputComponents = 128;
    resources.maxImageUnits = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resources.maxCombinedShaderOutputResources = 8;
    resources.maxImageSamples = 0;
    resources.maxVertexImageUniforms = 0;
    resources.maxTessControlImageUniforms = 0;
    resources.maxTessEvaluationImageUniforms = 0;
    resources.maxGeometryImageUniforms = 0;
    resources.maxFragmentImageUniforms = 8;
    resources.maxCombinedImageUniforms = 8;
    resources.maxGeometryTextureImageUnits = 16;
    resources.maxGeometryOutputVertices = 256;
    resources.maxGeometryTotalOutputComponents = 1024;
    resources.maxGeometryUniformComponents = 1024;
    resources.maxGeometryVaryingComponents = 64;
    resources.maxTessControlInputComponents = 128;
    resources.maxTessControlOutputComponents = 128;
    resources.maxTessControlTextureImageUnits = 16;
    resources.maxTessControlUniformComponents = 1024;
    resources.maxTessControlTotalOutputComponents = 4096;
    resources.maxTessEvaluationInputComponents = 128;
    resources.maxTessEvaluationOutputComponents = 128;
    resources.maxTessEvaluationTextureImageUnits = 16;
    resources.maxTessEvaluationUniformComponents = 1024;
    resources.maxTessPatchComponents = 120;
    resources.maxPatchVertices = 32;
    resources.maxTessGenLevel = 64;
    resources.maxViewports = 16;
    resources.maxVertexAtomicCounters = 0;
    resources.maxTessControlAtomicCounters = 0;
    resources.maxTessEvaluationAtomicCounters = 0;
    resources.maxGeometryAtomicCounters = 0;
    resources.maxFragmentAtomicCounters = 8;
    resources.maxCombinedAtomicCounters = 8;
    resources.maxAtomicCounterBindings = 1;
    resources.maxVertexAtomicCounterBuffers = 0;
    resources.maxTessControlAtomicCounterBuffers = 0;
    resources.maxTessEvaluationAtomicCounterBuffers = 0;
    resources.maxGeometryAtomicCounterBuffers = 0;
    resources.maxFragmentAtomicCounterBuffers = 1;
    resources.maxCombinedAtomicCounterBuffers = 1;
    resources.maxAtomicCounterBufferSize = 16384;
    resources.maxTransformFeedbackBuffers = 4;
    resources.maxTransformFeedbackInterleavedComponents = 64;
    resources.maxCullDistances = 8;
    resources.maxCombinedClipAndCullDistances = 8;
    resources.maxSamples = 4;
    resources.maxMeshOutputVerticesNV = 256;
    resources.maxMeshOutputPrimitivesNV = 512;
    resources.maxMeshWorkGroupSizeX_NV = 32;
    resources.maxMeshWorkGroupSizeY_NV = 1;
    resources.maxMeshWorkGroupSizeZ_NV = 1;
    resources.maxTaskWorkGroupSizeX_NV = 32;
    resources.maxTaskWorkGroupSizeY_NV = 1;
    resources.maxTaskWorkGroupSizeZ_NV = 1;
    resources.maxMeshViewCountNV = 4;
    resources.limits.nonInductiveForLoops = 1;
    resources.limits.whileLoops = 1;
    resources.limits.doWhileLoops = 1;
    resources.limits.generalUniformIndexing = 1;
    resources.limits.generalAttributeMatrixVectorIndexing = 1;
    resources.limits.generalVaryingIndexing = 1;
    resources.limits.generalSamplerIndexing = 1;
    resources.limits.generalVariableIndexing = 1;
    resources.limits.generalConstantMatrixVectorIndexing = 1;
}

bool toSPIRV(EShLanguage shader_type, string source, vector<uint32_t> &spv_code)
{
    glslang::InitializeProcess();

    glslang::TShader shader(shader_type);
    glslang::TProgram program;

    const char *shader_str[1];

    TBuiltInResource resources = {};
    initResources(resources);

    shader_str[0] = source.data();
    shader.setStrings(shader_str, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, shader_type, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    if (!shader.parse(&resources, 100, false, EShMsgDefault))
    {
	stringstream log_str;
	log_str << "Could not parse shader!" << endl;
	log_str << "Error log: " << endl;
	log_str << shader.getInfoLog() << endl;
	log_str << shader.getInfoDebugLog();
	cout << log_str.str() << endl;
	return false;
    }

    program.addShader(&shader);

    if (!program.link(EShMsgDefault))
    {
	stringstream log_str;
	log_str << "Could not link shader program!" << endl;
	log_str << "Error log: " << endl;
	log_str << shader.getInfoLog() << endl;
	log_str << shader.getInfoDebugLog();
	cout << log_str.str() << endl;
	return false;
    }

    if (!program.mapIO())
    {
	stringstream log_str;
	log_str << "Could not map shader program I/O!" << endl;
	log_str << "Error log: " << endl;
	log_str << shader.getInfoLog() << endl;
	log_str << shader.getInfoDebugLog();
	cout << log_str.str() << endl;
	return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(shader_type), spv_code);
    glslang::FinalizeProcess();
    return true;
}

bool toHLSL(vector<uint32_t> spv_code, bool is_d3d12, string &out_hlsl)
{
    CompilerHLSL compiler(spv_code);
    CompilerHLSL::Options hlsl_options;
    hlsl_options.shader_model = (is_d3d12) ? 50 : 40;
    compiler.set_hlsl_options(hlsl_options);

    out_hlsl = compiler.compile();

    if (out_hlsl.empty())
    {
	cout << "Could not compile shader to HLSL!" << endl;
	return false;
    }

    return true;
}

bool toGLSL(vector<uint32_t> spv_code, GLSLShaderLang shader_lang, string &out_glsl)
{
    CompilerGLSL compiler(spv_code);
    CompilerGLSL::Options glsl_options;

    switch (shader_lang)
    {
	case GLSL330:
	{
	    glsl_options.version = 330;
	    glsl_options.es = false;
	}
	break;
	case GLSL300ES:
	{
	    glsl_options.version = 300;
	    glsl_options.es = true;
	}
	break;
	default:
	{
	    cout << "Unrecognized GLSL version type of " << dec << int(shader_lang) << endl;
	    return false;
	}
	break;
    }

    glsl_options.emit_line_directives = false;
    glsl_options.vulkan_semantics = false;
    glsl_options.enable_420pack_extension = false;
    glsl_options.emit_uniform_buffer_as_plain_uniforms = true;
    compiler.set_common_options(glsl_options);

    out_glsl = compiler.compile();

    if (out_glsl.empty())
    {
	cout << "Could not compile shader to GLSL!" << endl;
	return false;
    }

    return true;
}

vector<string> fetchNamesGLSL(vector<uint32_t> spv_code)
{
    vector<string> glsl_names;
    CompilerGLSL compiler(spv_code);

    auto resources = compiler.get_shader_resources();

    for (auto &res : resources.stage_inputs)
    {
	glsl_names.push_back(res.name);
    }

    return glsl_names;
}

vector<pair<string, uint32_t>> fetchSemanticsHLSL(vector<uint32_t> spv_code)
{
    vector<pair<string, uint32_t>> hlsl_semantics;
    CompilerHLSL compiler(spv_code);

    auto resources = compiler.get_shader_resources();

    for (auto &res : resources.stage_inputs)
    {
	uint32_t slot = compiler.get_decoration(res.id, spv::DecorationLocation);
	hlsl_semantics.push_back(make_pair("TEXCOORD", slot));
    }

    return hlsl_semantics;
}

vector<uint32_t> fetchLocationsSPIRV(vector<uint32_t> spv_code)
{
    vector<uint32_t> spirv_locations;
    Compiler compiler(spv_code);

    auto resources = compiler.get_shader_resources();

    for (auto &res : resources.stage_inputs)
    {
	uint32_t location = compiler.get_decoration(res.id, spv::DecorationLocation);
	spirv_locations.push_back(location);
    }

    return spirv_locations;
}