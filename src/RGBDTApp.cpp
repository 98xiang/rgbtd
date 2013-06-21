#include <ctime>
#include <iostream>

#include "OpenNICamera.h"
#include "Camera.h"
#include "LBPFeature.h"
#include "CDTSensor.h"

using namespace std;

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", &tstruct);
    return buf;
}

void print_help()
{
	cerr << "CDTApp usage:" << endl;

	cerr << " -ip : specify input data path (use instead of the real sensor input)" << endl;
	cerr << " -op : specify output data path [deafult: .\\data\\dataset_xxx]" << endl;
	cerr << " -si : save images" << endl;
	cerr << " -sf : save features" << endl;
	cerr << " -pd : pyramid depth [deafult: 0]" << endl;
	cerr << " -sk : skip x out of xx frame when writing [deafult: 1 1]" << endl;
	cerr << " -vi : visualise images" << endl;
	cerr << " -vf : visualise features" << endl;
}

int main(int argc, char **argv)
{
	string input_data_path = "";
	string output_data_path = ".\\data\\dataset_" + currentDateTime() + "\\";
	bool save_images = false;
	bool save_features = false;
	bool show_images = false;
	bool show_feature_images = false;
	bool use_color = true, use_depth = true, use_thermal = true;
	int pyramid_depth = 0;
	LinLib::CDTFile image_writer, feature_writer;

	if (argc == 1)
	{
		print_help();
		exit(0);
	}
	
	for (int i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i],"-ip")==0) && (i < (argc-1))) { input_data_path = argv[++i]; }
		else if ((strcmp(argv[i],"-op")==0) && (i < (argc-1))) { output_data_path = argv[++i]; }
		else if ((strcmp(argv[i],"-pd")==0) && (i < (argc-1))) { pyramid_depth = atoi(argv[++i]); }
		else if (strcmp(argv[i],"-si")==0) { save_images = true; }
		else if (strcmp(argv[i],"-sf")==0) { save_features = true; }
		else if (strcmp(argv[i],"-vi")==0) { show_images = true; }
		else if (strcmp(argv[i],"-vf")==0) { show_feature_images = true; }
		else if (strcmp(argv[i],"-h")==0)	{ print_help();	}
		else if ((strcmp(argv[i],"-sk")==0) && (i < (argc-2))) 
		{
			image_writer.SkipFrames(atoi(argv[++i]));
			feature_writer.SkipFrames(atoi(argv[i]));
			image_writer.SkipOutOfFrames(atoi(argv[++i]));
			feature_writer.SkipOutOfFrames(atoi(argv[i]));
		}
	}

	LinLib::CDTSensor *input_device;

	if (input_data_path.compare("")!=0) //path speficied use images instead of the sensor
	{
		input_device = new LinLib::CDTFile();
		((LinLib::CDTFile*)input_device)->Path(input_data_path + "images\\");
	}
	else // use sensor
	{
		input_device = new LinLib::CDTDevice();
	}

	image_writer.Path(output_data_path + "images\\");
	feature_writer.Path(output_data_path + "features\\");

	LinLib::LBPFeature feature;

	try { input_device->Init(); }
	catch (LinLib::Exception*) { cerr << "Could not initialise the input device." << endl; exit(1); }

	while (cvWaitKey(1) != 27)
	{
		try { input_device->GrabAllImages(); }
		catch (LinLib::Exception*) { break; }

		if (save_images)
			image_writer.SaveImages(input_device->ColorFrame(), input_device->DepthFrame(), input_device->ThermalFrame());

		if (save_features)
			feature_writer.SaveFeatures(feature.Get(input_device->ColorFrame(), pyramid_depth), feature.Get(input_device->DepthFrame(), pyramid_depth), feature.Get(input_device->ThermalFrame(), pyramid_depth));

		if (show_images)
		{
			cv::imshow("color", input_device->ColorFrame());
			cv::imshow("depth", input_device->DepthFrame()*65535/10000);
			cv::imshow("thermal", input_device->ThermalFrame());
		}

		if (show_feature_images)
		{
			cv::imshow("color feature image", feature.GetFeatureImage(input_device->ColorFrame()));
			cv::imshow("depth feature image", feature.GetFeatureImage(input_device->DepthFrame()));
			cv::imshow("thermal feature image", feature.GetFeatureImage(input_device->ThermalFrame()));
		}

	}

	return 0;
}
