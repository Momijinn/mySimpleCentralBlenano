//BLE nano1
// #include <BLE_API.h>

//BLE nano 2
#include <nRF5x_BLE_API.h>

BLE           ble;

//接続したいPeripheral側のデバイス名
const char* FindDeviceName = "MyBlePeripheral";

static const uint8_t service_uuid[]       = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t service_chars_uuid[] = {0x71, 0x3D, 0, 1, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};


//Characteristicを発見したときにUUIDを格納する変数
DiscoveredCharacteristic chars_uuids;
//Peripheral側の情報を格納
DiscoveredCharacteristicDescriptor desc_of_chars_hrm(NULL,GattAttribute::INVALID_HANDLE,GattAttribute::INVALID_HANDLE,UUID::ShortUUIDBytes_t(0));

void ScanCallBack(const Gap::AdvertisementCallbackParams_t *params);
void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars);
void discoveryTerminationCallBack(Gap::Handle_t connectionHandle);
void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params);
void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) ;


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
 * 周辺のBLEをスキャニング
 */
void ScanCallBack(const Gap::AdvertisementCallbackParams_t *params) {
  Serial.println("Scan CallBack ");

  //BDアドレス
  Serial.print("PerrAddress: ");
  for( uint8_t index=0; index<6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Serial.print("The Rssi : ");
  // Serial.println(params->rssi, DEC);

  uint8_t len;
  uint8_t adv_name[31];


  //Peripheralの正式名
  if( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    Serial.print("Device name is : ");
    Serial.println((const char*)adv_name);
    
    //接続したいデバイスがあったら接続
    if(memcmp(FindDeviceName, adv_name,  strlen(FindDeviceName)) == 0x00) {
        Serial.println("find device");
        ble.stopScan(); //Scanをやめる

        //デバイスへ接続
        //connectionCallBack()へ行く
        ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
    }
  }
  Serial.println();
}


/**
 * 接続CallBack
 */
void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  Serial.println("\r\n----Connection CallBack");

  // ほしいServiceの中にあるCharacteristicを発見する
  // そのあとdiscoveredCharacteristicCallBackを呼びUUIDを取得する
  ble.gattClient().launchServiceDiscovery(params->handle, NULL, discoveredCharacteristicCallBack, service_uuid, service_chars_uuid);
}


/**
 * 切断処理
 */
void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.println("\r\n----Disconnected, restart to scanning");
  ble.startScan(ScanCallBack);
}


/**
 * ほしいCharacteristicの発見したときのcallback
 */
void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) {
  Serial.println("\r\n----Characteristic Discovered");

  //CharacteristicUUIDsをchars_uuidsに格納
  chars_uuids = *chars;
}


/**
 * デバイスとペアリングした後の処理
 * Notifyを受け取る準備をする
 */
void discoveryTerminationCallBack(Gap::Handle_t connectionHandle) {
  Serial.println("\r\n----discoveryTermination");

  ble.gattClient().discoverCharacteristicDescriptors(chars_uuids, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
}


/**
 * Peripheral側のnotifyにCCCD(0x2902)があるかのチェック
 */
void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) {
  Serial.println("\r\n----discovered descriptor");

  //
  if(params->descriptor.getUUID().getShortUUID() == 0x2902) { //Notification or indications disabled
    desc_of_chars_hrm = params->descriptor;
  }
}


/**
 * 通知を受け取る準備
 */
void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) {
  Serial.println("\r\n----Open notify");

  //Peripheral側に0x01を送信し，Notifyデータを送ってもらうようにする
  uint16_t value = 0x01;
  ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_uuids.getConnectionHandle(), desc_of_chars_hrm.getAttributeHandle(), 1, (uint8_t *)&value);
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

  ble.init(); //BLEの初期化
  ble.onConnection(connectionCallBack); //接続したときの処理
  ble.onDisconnection(disconnectionCallBack); //切断したときの処理
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallBack); //serviceの発見した後の処理
  ble.gattClient().onHVX(NotifyCallBack); //更新(notify)イベント

  // scan interval : in milliseconds, valid values lie between 2.5ms and 10.24s
  // scan window :in milliseconds, valid values lie between 2.5ms and 10.24s
  // timeout : in seconds, between 0x0001 and 0xFFFF, 0x0000 disables timeout
  // activeScanning : true or false
  ble.setScanParams(1000, 200, 0, true); //trueにしないと自動でリスキャンしない

  // start scanning
  ble.startScan(ScanCallBack); //Peripheralのスキャニングイベント
}

void loop() {
  // put your main code here, to run repeatedly:
  ble.waitForEvent();
}