  #include <include/hough_circles.h>

bool sort_len(const std::string &i, const std::string &j)
{
    return i.length() < j.length();
}

float extern numberOfImagesFound =  0;
float extern totalNumberOfImages = 0;

void compareSignAndResult(const int sign, const std::string &result)
{
    if(!result.empty())
    {
        if(std::stoi(result) == sign)
        {
            std::cout << "Найден. Номер знака: " << result << std::endl;
            ++numberOfImagesFound;
        }
        else
            std::cout << "НЕ РАСПОЗНАНО. Номер знака: " << result << std::endl;
    }
    else
    {
        std::cout << "НЕ НАЙДЕН." << std::endl;
    }
    ++totalNumberOfImages;
}

const std::string keys =
    "{ help  h     | | Print help message. }"
    "{ mode  m     | | Path to image, path directory. }"
    "{ input i     | | Path to input image. }"
    "{ input d     | | Path directory with images. }"
    "{ sign        | | Sign road. }"
    "{ width       | 200 | Preprocess input image by resizing to a specific width. It should be multiple by 32. }"
    "{ height      | 200 | Preprocess input image by resizing to a specific height. It should be multiple by 32. }";

int main(int argc, char** argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Recognition Of Road Text Signs");

    if (parser.has("help"))
    {
        parser.printMessage();
        return EXIT_SUCCESS;
    }

    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }

    const cv::String mode = parser.get<cv::String>("mode");
    const int sign = parser.get<int>("sign");
    const int imageWidth = parser.get<int>("width");
    const int imageHeight = parser.get<int>("height");
    std::string pathToImage = parser.get<std::string>("input");
    std::replace(pathToImage.begin(), pathToImage.end(), '\\', '/');

   HoughCircles roadSign(imageWidth, imageHeight);

    if(mode == "image")
    {
        auto timer_start_image = std::chrono::high_resolution_clock::now();

        std::string result {roadSign.recognitionText(pathToImage)};

        auto timer_end_image = std::chrono::high_resolution_clock::now();
        std::cout << "Timer for image: " << (double)std::chrono::duration_cast<std::chrono::milliseconds>(timer_end_image - timer_start_image).count() / 1000 << " sec" << std::endl;

        compareSignAndResult(sign, result);
    }
    else if(mode == "directory")
    {
        std::vector <std::string> names {};
        char filename[100] {};
        struct dirent *File;
        std::string directory {};
        directory = parser.get<std::string>("input");

        if(*directory.rbegin() != '/')
            directory.append("/");

        DIR *fullDirectory;
        fullDirectory = opendir(directory.c_str());

        while((File=readdir(fullDirectory)) != NULL)
        {
            if (strcmp(File->d_name, ".DS_Store") == 0)
            {
                continue;
            }

            if (File->d_type == DT_REG)
            {
                names.push_back(File->d_name);
            }
        }

        std::stable_sort(names.begin(), names.end(), sort_len);

        for(std::vector <std::string>::iterator iterName = names.begin(); iterName != names.end(); ++iterName)
        {
            sprintf(filename, (directory + "%s").c_str(), (*iterName).c_str());

            auto timer_start_image = std::chrono::high_resolution_clock::now();

            std::string result {roadSign.recognitionText(filename)};

            auto timer_end_image = std::chrono::high_resolution_clock::now();
            std::cout << "Timer for image: " << (double)std::chrono::duration_cast<std::chrono::milliseconds>(timer_end_image - timer_start_image).count() / 1000 << " sec" << std::endl;

            compareSignAndResult(sign, result);
        }

        closedir(fullDirectory);
    }

    std::cout << "Процент распознанных изображений: " << numberOfImagesFound / totalNumberOfImages * 100  << " %" << std::endl;
    return 0;
}
