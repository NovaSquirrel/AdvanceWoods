    .align 2                    @ Align to word boundary
    .thumb_func                 @ This is a thumb function
    .global thumb_test          @ This makes it a real symbol
    .type thumb_test STT_FUNC   @ Declare m5_plot_thumb to be a function.
thumb_test:                     @ Start of function definition
    bx      lr
