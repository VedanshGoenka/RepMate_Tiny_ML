#include "ble.h"
#include <string>

BLEAdvertisedDevice *myDevice = nullptr;
BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;
bool doConnect = false;
bool connected = false;
bool doScan = false;

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  printf("Notify callback for characteristic %s of data length %d\n", 
         pBLERemoteCharacteristic->getUUID().toString().c_str(), length);
  printf("data: ");
  fwrite(pData, 1, length, stdout);
  printf("\n");
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    printf("onDisconnect\n");
  }
};

bool connectToServer() {
  printf("Forming a connection to %s\n", myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  printf(" - Created client\n");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  printf(" - Connected to server\n");
  pClient->setMTU(517);  // set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    printf("Failed to find our service UUID: %s\n", serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  printf(" - Found our service\n");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    printf("Failed to find our characteristic UUID: %s\n", charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  printf(" - Found our characteristic\n");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    printf("The characteristic value was: %s\n", value.c_str());
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }

  connected = true;
  return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    printf("BLE Advertised Device found: %s\n", advertisedDevice.toString().c_str());

    if (advertisedDevice.haveName() && advertisedDevice.getName() == "RepMate") {
      printf("Found the correct device by name!\n");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
    }

    if (advertisedDevice.haveServiceUUID()) {
        printf("Advertised service UUID: %s\n", advertisedDevice.getServiceUUID().toString().c_str());
    }
    
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      printf("Target service UUID found!\n");
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

void BLEsetup() {
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void BLEloop(String message) {
  if (doConnect == true) {
    if (connectToServer()) {
      printf("We are now connected to the BLE Server.\n");
    } else {
      printf("We have failed to connect to the server; there is nothing more we will do.\n");
    }
    doConnect = false;
  }

  if (connected) {
    printf("Setting Lift History to: \"%s\"\n", message.c_str());
    pRemoteCharacteristic->writeValue(message.c_str(), message.length());
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }
}