project('city-gen', 'c')

add_project_arguments('-DWall', language : 'c')

subdir('src')
subdir('include')

deps = []
deps += dependency('sdl2')
deps += dependency('opengl')
deps += dependency('glew')



executable('city-gen', sources, dependencies: deps, include_directories: inc_dir)
