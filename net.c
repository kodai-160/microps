/* NOTE: if you want to add/delete the entries after net_run(), you need to protect these lists with a mutex. */
static struct net_device *devices;                                //デバイスリスト(リストの先頭を指すポインタ)

struct net_device *
net_device_alloc(void)
{
    struct net_device *dev;

    dev = memory_alloc(sizeof(*dev));
    if (!dev) {
        errorf("memory_alloc() failure");
        return NULL;
    }
    return dev;
};

/* NOTE: must not be call after net_run() */
int
net_device_register(struct net_device *dev)
{
    static unsigned int index = 0;

    dev->index = index++;                                           // デバイスのインデックス番号を設定
    sprintf(dev->name, sizeof(dev->name), "net%d", dev->index);     // デバイス名を生成
    dev->next = devices;                                            // デバイスリストの先頭に追加
    devices = dev;                                                  // デバイスリストの先頭に追加
    infof("registerd, dev=%s, type=0x%04x", dev->name, dev->type);
    return 0;
};

static int
net_device_open(struct net_device *dev)
{
    if (NET_DEVICE_IS_UP(dev)) {                           //デバイスの状態を確認(既にUP状態の場合はエラーを返す)
        errorf("already opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->open) {                                  //デバイスドライバのopen関数を呼び出す
        if (dev->ops->open(dev) == -1) {                   //open関数が設定されていない場合は呼び出しをスキップ
            errorf("failure, dev=%s", dev->name);          //エラーが返されたらこの関数もエラーを返す
            return -1;
        }
    }
    dev->flags |= NET_DEVICE_FLAG_UP;                      //UPフラグを立てる
    infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
}

static int
net_device_close(struct net_device *dev)
{
    if (!NET_DEVICE_IS_UP(dev)) {                          //デバイスの状態を確認(UPではない場合はエラーを返す)
        errorf("not opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->close) {                                 //デバイスドライバのopen関数を呼び出す
        if (dev->ops->close(dev) == -1) {                   //open関数が設定されていない場合は呼び出しをスキップ
            errorf("failure, dev=%s", dev->name);           //エラーが返されたらこの関数もエラーを返す
            return -1;
        }
    }
    dev->flags &= ~NET_DEVICE_FLAG_UP;                     //UPフラグを落とす
    infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
};