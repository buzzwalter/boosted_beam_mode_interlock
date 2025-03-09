#include <pylon/PylonIncludes.h>

// using namespace Pylon;
// using namespace std;

int main()
{
  // Initialize the Pylon runtime
  Pylon::PylonInitialize();

  try {
    // Get the transport layer factory
    Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

    // Get all attached devices
    Pylon::DeviceInfoList_t devices;
    if (tlFactory.EnumerateDevices(devices) == 0) {
      std::cout << "No camera found. Please check your USB connection." << std::endl;
      Pylon::PylonTerminate();
      return -1;
    }

    // Print device info of all cameras
    std::cout << "Found " << devices.size() << " camera(s):" << std::endl;
    std::cout << "=========================" << std::endl;

    for (size_t i = 0; i < devices.size(); i++) {
      std::cout << "Camera " << i << ":" << std::endl;
      std::cout << "  Device Model: " << devices[i].GetModelName() << std::endl;
      std::cout << "  Serial Number: " << devices[i].GetSerialNumber() << std::endl;
      std::cout << "  Device Vendor: " << devices[i].GetVendorName() << std::endl;
      std::cout << "  Device Type: " << devices[i].GetDeviceClass() << std::endl;
      std::cout << "------------------------" << std::endl;
    }

    // Create a camera object for the first available camera
    Pylon::CInstantCamera camera(tlFactory.CreateDevice(devices[0]));

    // Open the camera
    std::cout << "Opening camera..." << std::endl;
    camera.Open();

    if (camera.IsOpen()) {
      std::cout << "Camera open successful!" << std::endl;

      // Optional: Try to access some camera parameters
      // std::cout << "Camera information:" << std::endl;
      //      std::cout << "  Width: " << camera.Width.GetValue() << std::endl;
      //      std::cout << "  Height: " << camera.Height.GetValue() << std::endl;

      // Close the camera
      camera.Close();
      std::cout << "Camera closed." << std::endl;
    } else {
      std::cout << "Failed to open camera." << std::endl;
    }
  }
  catch (const std::exception &e) {
    std::cerr << "An exception occurred: " << e.what() << std::endl;
  }

  // Clean up
  Pylon::PylonTerminate();

  return 0;
}
