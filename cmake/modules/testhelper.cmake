function(copyTestToStaging testTarget)
    add_custom_command(TARGET "${testTarget}"
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${testTarget}"
            "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest"
        )
endfunction()


function(copyTestDataToStaging testTarget dataDir)
    copyDirIfDifferent("${testTarget}" POST_BUILD
            "${CMAKE_CURRENT_SOURCE_DIR}/${dataDir}"
            "${CMAKE_BINARY_DIR}/python/libavg/test/cpptest/baseline")
endfunction()

function(copyToStaging dir)
    set(python_test_package_dir "${CMAKE_BINARY_DIR}/python/libavg/test")
    copyDirIfDifferent(copy_python_tests POST_BUILD
            "${CMAKE_CURRENT_SOURCE_DIR}/${dir}"
            "${python_test_package_dir}/${dir}")
endfunction()

function(copyDirIfDifferent target stage src_dir dest_dir)
    add_custom_command(TARGET ${target}
        ${stage}
        COMMAND ${CMAKE_COMMAND}
                ARGS -E make_directory "${dest_dir}"
        )
    file(GLOB filelist
            RELATIVE "${src_dir}" "${src_dir}/*")
    foreach(cur_file ${filelist})
        add_custom_command(TARGET ${target}
            ${stage}
            COMMAND ${CMAKE_COMMAND}
            ARGS -E copy_if_different "${src_dir}/${cur_file}" "${dest_dir}/${cur_file}"
            )
    endforeach()
endfunction()
