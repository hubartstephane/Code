-- =============================================================================
-- ROOT_PATH/executables/linux
-- =============================================================================

for _, v in ipairs({"test1", "test_glm" })   do
	build:ProcessSubPremake(v, true) -- create a sub groups
end
