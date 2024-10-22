#======= project =======
cmake_minimum_required(VERSION 3.20)
project(
    PrinterDataCollector
	VERSION 0.0.1
	LANGUAGES CXX
)

#======= standard =======
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#======= libs =======
#find_package(snmp++ REQUIRED)

#======= target =======
add_executable(PrinterDataCollector)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
target_sources(PrinterDataCollector PRIVATE ${SRC})

target_link_libraries(PrinterDataCollector
    PRIVATE snmp++
)
