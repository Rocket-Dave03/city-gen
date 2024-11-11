project('city-gen', 'c')

subdir('src')
subdir('include')

deps = []
deps += dependency('sdl2')
deps += dependency('opengl')
deps += dependency('glew')

add_global_arguments('-DWall', language : 'c')

executable('city-gen', sources, dependencies: deps, include_directories: inc_dir)
