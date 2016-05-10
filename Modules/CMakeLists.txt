set(module_dirs
 Segmentation
)

foreach(module_dir ${module_dirs})
  add_subdirectory(${module_dir})
endforeach()
