<script setup>
import { GetFile, PutFile, SetRemoteIp, SelectWorkingDir, GetCurrentDir, GetLastError, ReadLogFile, GetLastSpeed } from '../../wailsjs/go/main/App'
import { useDialog } from 'naive-ui'
//onMount, watchEffect
import { onMounted, watchEffect, nextTick } from 'vue'

const dialog = useDialog()

let currentDir = $ref('')
let serverAddr = $ref('127.0.0.1')
let localFilename = $ref('')
let remoteFilename = $ref('')

let isRunning = $ref(false)
let percentage = $ref(30)

GetCurrentDir().then((dir) => {
    currentDir = dir
})

let onServerAddrChanged = () => {
    console.log('serverAddr', serverAddr)
    SetRemoteIp(serverAddr)
}

onServerAddrChanged()

let onDownload = () => {
    if (isRunning) {
        return
    }
    isRunning = true
    console.log('onDownload', localFilename, remoteFilename)
    // int get_file(char *filename, char *local_name, int mode)
    let mode = 1
    GetFile(remoteFilename, localFilename, mode)
        .then((res) => {
            if (res == 0) {
                dialog.success({
                    title: '下载',
                    content: '下载成功',
                    positiveText: "确定",
                })
            } else {
                let err = GetLastError()
                dialog.error({
                    title: '下载',
                    content: err,
                    positiveText: "确定",
                })
            }
            console.log(res)
        })
        .catch((err) => {
            console.log(err)
        })
        .finally(() => {
            isRunning = false
        })
}

let onUpload = () => {
    if (isRunning) {
        return
    }
    isRunning = true
    console.log('onUpload', localFilename, remoteFilename)
    let mode = 1
    PutFile(localFilename, remoteFilename, mode)
        .then((res) => {
            if (res == 0) {
                dialog.success({
                    title: '上传',
                    content: '上传成功',
                    positiveText: "确定",
                })
            } else {
                GetLastError().then((err) => {
                    dialog.error({
                        title: '上传',
                        content: err,
                        positiveText: "确定",
                    })
                })
            }
        })
        .catch((err) => {
            console.log(err)
        }).finally(() => {
            isRunning = false
        })
}

let onChangeWorkingDir = () => {
    SelectWorkingDir()
        .then((res) => {
            console.log(res)
            currentDir = res
        })
        .catch((err) => {
            console.log(err)
        })
}


let log = $ref('')
let showModal = $ref(false)

const logInst = $ref(null)
onMounted(() => {
    watchEffect(() => {
        if (log) {
            nextTick(() => {
                logInst?.scrollTo({ position: "bottom", slient: true });
            });
        }
    });
});

let onWatchLog = () => {
    ReadLogFile()
        .then((res) => {
            // dialog.success({
            //     title: '日志',
            //     content: res,
            //     positiveText: "确定",
            // })
            showModal = true
            log = res
        })
        .catch((err) => {
            console.log(err)
        })
}

let speed = $ref("0.00 KB/s")
setInterval(() => {
    GetLastSpeed().then((res) => {
        speed = res
    })
}, 1000)

</script>

<template>
    <div class="px-3 py-2">
        <div>
            <div class="py-2 font-medium">当前工作目录</div>
            <n-input-group>
                <n-input v-model:value="currentDir" type="text" />
                <n-button type="primary" @click="onChangeWorkingDir">
                    修改
                </n-button>
            </n-input-group>
        </div>
        <div>
            <div class="py-2 font-medium">服务器地址</div>
            <n-input v-model:value="serverAddr" @blur="onServerAddrChanged" type="text" />
        </div>
        <div>
            <div class="py-2 font-medium">本地文件</div>
            <n-input-group>
                <n-input v-model:value="localFilename" type="text" />
                <n-button type="primary">
                    修改
                </n-button>
            </n-input-group>
        </div>
        <div>
            <div class="py-2 font-medium">远程文件</div>
            <n-input v-model:value="remoteFilename" type="text" />
        </div>
        <div class="py-4 flex flex-row justify-evenly">
            <n-button @click="onDownload">下载文件</n-button>
            <n-button @click="onUpload">上传文件</n-button>
            <n-button @click="onWatchLog">查看日志</n-button>
        </div>
        <!-- <n-progress type="line" :percentage="percentage" :indicator-placement="'inside'" /> -->
        <div class="flex flex-row justify-center py-1">
            <svg class="px-1" style="width: 2em" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2691">
                <path
                    d="M568.96 459.242667l144-189.205334A382.208 382.208 0 0 0 512 213.333333c-212.074667 0-384 171.925333-384 384a382.293333 382.293333 0 0 0 93.397333 251.008l-64.554666 55.808A467.584 467.584 0 0 1 42.666667 597.333333C42.666667 338.133333 252.8 128 512 128c93.098667 0 179.861333 27.093333 252.842667 73.856l75.882666-99.690667 67.541334 51.413334-273.28 359.04a149.333333 149.333333 0 1 1-66.048-53.376z m266.453333-69.056l54.357334-71.424A467.242667 467.242667 0 0 1 981.333333 597.333333c0 113.706667-40.64 221.226667-113.237333 305.728l-64.725333-55.616A382.272 382.272 0 0 0 896 597.333333c0-76.288-22.250667-147.370667-60.586667-207.146666zM512 661.333333a64 64 0 1 0 0-128 64 64 0 0 0 0 128z"
                    p-id="2692"></path>
            </svg>
            {{ speed }}
        </div>
    </div>
    <n-modal v-model:show="showModal">
        <n-card style="width: 600px" title="日志" :bordered="false" size="huge" role="dialog" aria-modal="true">
            <n-log ref="logInst" :log="log" trim />
        </n-card>
    </n-modal>
</template>

<style>
</style>
