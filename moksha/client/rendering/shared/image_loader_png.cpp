// DEPRECATED FILE: Using platform specific image loading now.

// #include "image_loader_png.h"

// #include <png.h>

// #include <csetjmp>
// #include <cstdio>
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <vector>

// #include "moksha/shared/misc/utils.h"

// // -- -Libpng Error Handling Callbacks-- -
// // These are required by libpng.
// void user_error_fn(png_structp png_ptr, png_const_charp error_msg) {
//     std::cerr << "Libpng Error: " << error_msg << std::endl;
//     // Retrieve the jump buffer structure associated with png_ptr
//     jmp_buf* jmp_buf_ptr = (jmp_buf*)png_get_error_ptr(png_ptr);
//     if (jmp_buf_ptr) {
//         longjmp(*jmp_buf_ptr, 1);  // Jump back to the setjmp point
//     } else {
//         // Handle cases where the error pointer wasn't set (shouldn't happen with default setup)
//         std::cerr << "Libpng error: Fatal error, no jump buffer configured." << std::endl;
//         abort();  // Terminate in case of unexpected error state
//     }
// }

// void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
//     std::cerr << "Libpng Warning: " << warning_msg << std::endl;
//     // Warnings don't usually require jumping, just logging.
// }

// bool ME::LoadPNG(const char* fileName, PNGData& out_data) {
//     std::string fileNameString = ME::Utils::GetResourcesPath() + fileName;
//     const char* filename = fileNameString.c_str();

//     png_structp png_ptr = nullptr;
//     png_infop info_ptr = nullptr;
//     FILE* fp = nullptr;
//     png_byte header[8];                 // PNG signature size
//     png_bytep* row_pointers = nullptr;  // Array of pointers to rows in image buffer

//     // 1. Open the file
// #ifdef _WIN32
//     fopen_s(&fp, filename, "rb");  // Use secure version on Windows
// #else
//     fp = fopen(filename, "rb");
// #endif
//     if (!fp) {
//         std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
//         return false;
//     }

//     // 2. Check PNG signature
//     if (fread(header, 1, 8, fp) != 8) {
//         std::cerr << "Error: Could not read PNG signature from '" << filename << "'" << std::endl;
//         fclose(fp);
//         return false;
//     }
//     if (png_sig_cmp(header, 0, 8) != 0) {
//         std::cerr << "Error: File '" << filename << "' is not a valid PNG." << std::endl;
//         fclose(fp);
//         return false;
//     }

//     // 3. Create libpng read and info structures
//     // We pass our error handlers and a pointer to our jump buffer for error handling.
//     // Note: We don't pass the jump buffer directly here, but ensure png_get_error_ptr works in error_fn.
//     png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, user_error_fn, user_warning_fn);
//     if (!png_ptr) {
//         std::cerr << "Error: png_create_read_struct failed." << std::endl;
//         fclose(fp);
//         return false;
//     }

//     info_ptr = png_create_info_struct(png_ptr);
//     if (!info_ptr) {
//         std::cerr << "Error: png_create_info_struct failed." << std::endl;
//         png_destroy_read_struct(&png_ptr, nullptr, nullptr);
//         fclose(fp);
//         return false;
//     }

//     // 4. Set up error handling using setjmp/longjmp
//     jmp_buf jmpbuf;
//     if (setjmp(jmpbuf)) {
//         // This code is executed if longjmp is called from the error handler
//         std::cerr << "Error: Libpng encountered an error during processing." << std::endl;
//         png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
//         if (row_pointers) free(row_pointers);  // Clean up row pointers if allocated
//         fclose(fp);
//         return false;
//     }
//     // Associate the jump buffer with the png_ptr so error_fn can use it
//     png_set_error_fn(png_ptr, &jmpbuf, user_error_fn, user_warning_fn);

//     // 5. Initialize I/O
//     png_init_io(png_ptr, fp);
//     // 6. Indicate we already read the 8 signature bytes
//     png_set_sig_bytes(png_ptr, 8);

//     // 7. Read PNG info (metadata)
//     png_read_info(png_ptr, info_ptr);

//     // 8. Get image dimensions and properties
//     out_data.width = png_get_image_width(png_ptr, info_ptr);
//     out_data.height = png_get_image_height(png_ptr, info_ptr);
//     png_byte color_type = png_get_color_type(png_ptr, info_ptr);
//     png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

//     // 9. Apply transformations for consistent output (e.g., aim for 8-bit RGBA)
//     // Expand palette to RGB
//     if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);

//     // Ensure grayscale is 8-bit
//     if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);

//     // Convert transparency chunk to full alpha channel
//     if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

//     // Reduce 16-bit channels to 8-bit
//     if (bit_depth == 16) png_set_strip_16(png_ptr);

//     // Add alpha channel if missing (convert RGB/Gray to RGBA/GA)
//     // This makes the output consistently 4 or 2 channels for simplicity downstream
//     if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type ==
//     PNG_COLOR_TYPE_PALETTE)
//         png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);  // Add 0xFF alpha

//     // Convert grayscale (with or without alpha) to RGB (so output is always RGB or RGBA)
//     if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);

//     // Apply transformations
//     png_read_update_info(png_ptr, info_ptr);

//     // After transformations, check final color type and channels
//     out_data.channels = png_get_channels(png_ptr, info_ptr);
//     png_byte final_color_type = png_get_color_type(png_ptr, info_ptr);
//     out_data.has_alpha = (final_color_type == PNG_COLOR_TYPE_RGBA);  // Check final type

//     // 10. Calculate row size & allocate memory for the pixel buffer
//     size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
//     out_data.pixels.resize(row_bytes * out_data.height);

//     // 11. Allocate row pointers array
//     // This is an array of pointers, where each pointer points to the start of a row in out_data.pixels
//     row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * out_data.height);
//     if (!row_pointers) {
//         std::cerr << "Error: Failed to allocate memory for row pointers." << std::endl;
//         longjmp(jmpbuf, 1);  // Use longjmp to trigger cleanup
//     }
//     for (int y = 0; y < out_data.height; ++y) {
//         row_pointers[y] = out_data.pixels.data() + y * row_bytes;
//     }

//     // 12. Read the image data
//     png_read_image(png_ptr, row_pointers);

//     // 13. Clean up
//     free(row_pointers);  // Free the row pointers array
//     png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
//     fclose(fp);

//     return true;
// }