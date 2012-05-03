#include "vpxt_test_declarations.h"

int test_copy_set_reference(int argc,
                            const char** argv,
                            const std::string &working_dir,
                            const std::string sub_folder_str,
                            int test_type,
                            int delete_ivf,
                            int artifact_detection)
{
    char *comp_out_str = "Error Resilient";
    char *test_dir = "test_copy_set_reference";
    int input_ver = vpxt_check_arg_input(argv[1], argc);

    if (input_ver < 0)
        return vpxt_test_help(argv[1], 0);

    std::string input = argv[2];
    int mode = atoi(argv[3]);
    int bitrate = atoi(argv[4]);
    int first_copy_frame = atoi(argv[5]);
    std::string enc_format = argv[6];
    std::string dec_format = argv[7];

    int speed = 0;
    int test_state = kTestFailed;

    //////////// Formatting Test Specific directory ////////////
    std::string cur_test_dir_str;
    std::string file_index_str;
    char main_test_dir_char[255] = "";
    char file_index_output_char[255] = "";

    if (initialize_test_directory(argc, argv, test_type, working_dir, test_dir,
        cur_test_dir_str, file_index_str, main_test_dir_char,
        file_index_output_char, sub_folder_str) == 11)
        return kTestErrFileMismatch;

    std::string copy_set_enc = cur_test_dir_str + slashCharStr() + test_dir +
        "_compression";
    vpxt_enc_format_append(copy_set_enc, enc_format);

    std::string copy_set_dec = cur_test_dir_str + slashCharStr() + test_dir +
        "_decompression";
    vpxt_dec_format_append(copy_set_dec, dec_format);

    std::string copy_set_dec_2 = cur_test_dir_str + slashCharStr() + test_dir +
        "_decompression_clone";
    vpxt_dec_format_append(copy_set_dec_2, dec_format);

    ///////////// OutPutfile ////////////
    std::string text_file_str = cur_test_dir_str + slashCharStr() + test_dir;

    if (test_type == kCompOnly || test_type == kFullTest)
        text_file_str += ".txt";
    else
        text_file_str += "_TestOnly.txt";

    FILE *fp;

    if ((fp = freopen(text_file_str.c_str(), "w", stderr)) == NULL)
    {
        tprintf(PRINT_STD, "Cannot open out put file: %s\n",
            text_file_str.c_str());
        exit(1);
    }

    ////////////////////////////////
    //////////////////////////////////////////////////////////

    if (test_type == kFullTest)
        print_header_full_test(argc, argv, main_test_dir_char);

    if (test_type == kCompOnly)
        print_header_compression_only(argc, argv, main_test_dir_char);

    if (test_type == kTestOnly)
        print_header_test_only(argc, argv, cur_test_dir_str);

    vpxt_cap_string_print(PRINT_BTH, "%s", test_dir);

    VP8_CONFIG opt;
    vpxt_default_parameters(opt);

    /////////////////// Use Custom Settings ///////////////////
    if (input_ver == 2)
    {
        if (!vpxt_file_exists_check(argv[argc-1]))
        {
            tprintf(PRINT_BTH, "\nInput Settings file %s does not exist\n",
                argv[argc-1]);

            fclose(fp);
            record_test_complete(file_index_str, file_index_output_char,
                test_type);
            return kTestIndeterminate;
        }

        opt = vpxt_input_settings(argv[argc-1]);
        bitrate = opt.target_bandwidth;
    }

    /////////////////////////////////////////////////////////

    opt.target_bandwidth = bitrate;

    // Run Test only (Runs Test, Sets up test to be run, or skips compresion of
    // files)
    if (test_type == kTestOnly)
    {
        // This test requires no preperation before a Test Only Run
    }
    else
    {
        opt.Mode = mode;
        opt.error_resilient_mode = 1;

        if (vpxt_compress(input.c_str(), copy_set_enc.c_str(), speed, bitrate,
            opt, comp_out_str, opt.error_resilient_mode, 0, enc_format) == -1)
        {
            fclose(fp);
            record_test_complete(file_index_str, file_index_output_char,
                test_type);
            return kTestIndeterminate;
        }

        test_state = vpxt_decompress_copy_set(copy_set_enc.c_str(),
            copy_set_dec.c_str(), copy_set_dec_2.c_str(), dec_format, 1,
            first_copy_frame, PRINT_BTH);
    }

    if (test_type == kCompOnly)
    {
        fclose(fp);
        record_test_complete(file_index_str, file_index_output_char, test_type);
        return kTestEncCreated;
    }

    tprintf(PRINT_BTH, "\n\nResults:\n\n");

    if (test_state == kTestFailed)
    {
        vpxt_formated_print(RESPRT, "Not all cloned decompression frames "
            "identical to non cloned decompression frames - Failed");
        tprintf(PRINT_BTH, "\n");
    }

    if (test_state == kTestPassed)
    {
        vpxt_formated_print(RESPRT, "All cloned decompression frames identical "
            "to non cloned decompression frames - Passed");
        tprintf(PRINT_BTH, "\n");
    }

    if (test_state == kTestIndeterminate)
    {
        vpxt_formated_print(RESPRT, "First clone frame not reached - "
            "Indeterminate");
        tprintf(PRINT_BTH, "\n");
    }

    if (test_state == kTestPassed)
        tprintf(PRINT_BTH, "\nPassed\n");
    if (test_state == kTestIndeterminate)
        tprintf(PRINT_BTH, "\nIndeterminate\n");
    if (test_state == kTestFailed)
        tprintf(PRINT_BTH, "\nFailed\n");

    if (delete_ivf)
        vpxt_delete_files(3, copy_set_enc.c_str(), copy_set_dec.c_str(),
        copy_set_dec_2.c_str());

    fclose(fp);
    record_test_complete(file_index_str, file_index_output_char, test_type);
    return test_state;
}
