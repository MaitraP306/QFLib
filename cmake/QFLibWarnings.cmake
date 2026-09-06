function(qflib_set_warnings target)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        return()
    endif()

    target_compile_options(${target}
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
        )

    if(QFLIB_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE -Werror)
    endif()
endfunction()

function(qflib_enable_sanitizers target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target}
            PUBLIC
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        target_link_options(${target}
            PUBLIC
                -fsanitize=address,undefined
        )
    else()
        message(WARNING "Sanitizers are not configured for this compiler.")
    endif()
endfunction()
