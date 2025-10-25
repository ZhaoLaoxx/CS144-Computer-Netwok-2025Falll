file(GLOB LIB_SOURCES "*.cc")

# 添加源文件到已经定义的库中
target_sources(minnow_debug PRIVATE ${LIB_SOURCES})
target_sources(minnow_sanitized PRIVATE ${LIB_SOURCES})
target_sources(minnow_optimized PRIVATE ${LIB_SOURCES})