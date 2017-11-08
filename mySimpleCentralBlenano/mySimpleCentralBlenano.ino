//BLE nano1
// #include <BLE_API.h>

//BLE nano 2
#include <nRF5x_BLE_API.h>

BLE           ble;

const char* DeviceName = "MyBlePeripheral";
static const uint8_t service_uuid[]       = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t service_chars_uuid[] = {0x71, 0x3D, 0, 1, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};


//通知を受け取るのに必要そう
static DiscoveredCharacteristic            chars_hrm;
static DiscoveredCharacteristicDescriptor  desc_of_chars_hrm(NULL,GattAttribute::INVALID_HANDLE,GattAttribute::INVALID_HANDLE,UUID::ShortUUIDBytes_t(0));

static void ScanCallBack(const Gap::AdvertisementCallbackParams_t *params);
static void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars);
static void discoveryTerminationCallBack(Gap::Handle_t connectionHandle);
static void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params);
static void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) ;

/**
 * スキャニングしたデバイス名をデコード
 */
uint32_t ble_advdata_parser(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data) {
    uint8_t index=0;
    uint8_t field_length, field_type;

    while(index<advdata_len) {
      field_length = p_advdata[index];
      field_type   = p_advdata[index+1];
      if(field_type == type) {
        memcpy(p_field_data, &p_advdata[index+2], (field_length-1));
        *len = field_length - 1;
        return NRF_SUCCESS;
      }
      index += field_length + 1;
    }
    return NRF_ERROR_NOT_FOUND;
  }


/**
 * 発見後、serviceUUIDとキャラクタスタチックを突き合わせる
 * Serviceの接続確認は省略している
 */
void startDiscovery(uint16_t handle) {
  Serial.println("----startDiscovery");

  //Notifyのみほしいため
  ble.gattClient().launchServiceDiscovery(handle, NULL, discoveredCharacteristicCallBack, service_uuid, service_chars_uuid);
}


/**
 * 周辺のBLEをスキャニング
 */
static void ScanCallBack(const Gap::AdvertisementCallbackParams_t *params) {
  Serial.println("Scan CallBack ");

  Serial.print("PerrAddress: ");
  for( uint8_t index=0; index<6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("The Rssi : ");
  Serial.println(params->rssi, DEC);

  Serial.print("The adv_data : ");
  Serial.println((const char*)params->advertisingData);

  uint8_t len;
  uint8_t adv_name[31];
  if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    Serial.println("#1");

    Serial.print("Device name is : ");
    Serial.println((const char*)adv_name);

    //検索したいデバイスがあったら接続
    if(memcmp(DeviceName, adv_name,  strlen(DeviceName)) == 0x00) {
        Serial.println("find device");
        ble.stopScan();
        ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
    }


  }else if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    Serial.println("#2");

    Serial.print("Short Device name is : ");
    Serial.println((const char*)adv_name);

    if(memcmp(DeviceName, adv_name,  strlen(DeviceName)) == 0x00) {
      Serial.println("find device");
      ble.stopScan();
      ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
    }
  }
  Serial.println();
}


/**
 * 接続
 */
void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  uint8_t index;

  Serial.print("The conn handle : ");
  Serial.println(params->handle, HEX);

  Serial.print("  The peerAddr : ");
  for(index=0; index<6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  // start to discovery
  startDiscovery(params->handle);
}

/**
 * 切断処理
 */
void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.println("Disconnected, start to scanning");
  ble.startScan(ScanCallBack);
}


/**
 * ここでキャラクタスタティックの探索(突き合わせ)
 */
static void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) {
  Serial.println("\r\n----Characteristic Discovered");

  Serial.print("Chars UUID             : ");
  const uint8_t *uuid = chars->getUUID().getBaseUUID();
  for(uint8_t index=0; index<16; index++) { //反転しているため注意 characteristicがでてくる
    Serial.print(uuid[index], HEX);
    Serial.print(" ");
  }

  //キャラクタスタティックをもらう
  chars_hrm = *chars;
  Serial.println(" ");
}


/**
 * Notifyを受け取る準備
 */
static void discoveryTerminationCallBack(Gap::Handle_t connectionHandle) {
  Serial.println("\r\n----discoveryTermination");
  ble.gattClient().discoverCharacteristicDescriptors(chars_hrm, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
}


/**
 * Descriptiorsの検索
 */
static void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) {
  Serial.println("\r\n----discovered descriptor");
  Serial.print("Desriptor UUID         : ");
  Serial.println(params->descriptor.getUUID().getShortUUID(), HEX);

  if(params->descriptor.getUUID().getShortUUID() == 0x2902) { //Notification or indications disabled
    desc_of_chars_hrm = params->descriptor;
  }
}


/**
 * 通知を受け取る準備
 */
static void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) {
  Serial.println("\r\n----discovery descriptor Termination");

  Serial.println("Open notify");
  uint16_t value = 0x01;
  ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_hrm.getConnectionHandle(), desc_of_chars_hrm.getAttributeHandle(), 1, (uint8_t *)&value);
}



/**
 * 通知されたデータを受け取る
 */
void NotifyCallBack(const GattHVXCallbackParams *params) {
  Serial.println("GattClient notify call back ");

  for(unsigned char index=0; index<params->len; index++) {
    Serial.print(params->data[index], HEX);
  }
  Serial.println("");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("BLE Central Recv Notify Only");

  ble.init();
  ble.onConnection(connectionCallBack);
  ble.onDisconnection(disconnectionCallBack);
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallBack);
  ble.gattClient().onHVX(NotifyCallBack);

  // scan interval : in milliseconds, valid values lie between 2.5ms and 10.24s
  // scan window :in milliseconds, valid values lie between 2.5ms and 10.24s
  // timeout : in seconds, between 0x0001 and 0xFFFF, 0x0000 disables timeout
  // activeScanning : true or false
  ble.setScanParams(1000, 200, 0, true); //trueにしないとうまく接続してくれないっぽい

  // start scanning
  ble.startScan(ScanCallBack);
}

void loop() {
  // put your main code here, to run repeatedly:
  ble.waitForEvent();
}