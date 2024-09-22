-- =============================================================================
-- ROOT_PATH/executables/test_external_libraries
-- =============================================================================

for _, v in ipairs({"test_empty", "test_glm", "test_tinyxml2", "test_nlohmann", "test_glfw", "test_freeimage", "test_classes", "test_lua", "test_freetype2", "test_assimp", "test_glslang", "test_vulkan", "test_zlib", "test_boost", "test_irrklang", "test_imgui" })   do
	build:ProcessSubPremake(v, true) -- create a sub groups
end
