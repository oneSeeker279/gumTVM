// target so
var Module_name = "libmyapplication.so"
// target func offset
var target_offset = 0x2EBD4
// app package name
var package_name = "com.example.myapplication"
// trace file
var trace_file_name = "trace.txt"


function copyFile(srcPath, destPath){
    var O_RDONLY = 0; 
    var O_WRONLY = 1;
    var O_CREAT = 0x40;
    var O_TRUNC = 0x200;
    var S_IRWXU = 0x1C0; // 0700 权限（所有者可读写执行）
    var libc = Module.findExportByName("libc.so", "open");
    var read = Module.findExportByName("libc.so", "read");
    var write = Module.findExportByName("libc.so", "write");
    var close = Module.findExportByName("libc.so", "close");

    var open = new NativeFunction(libc, 'int', ['pointer', 'int', 'int']);
    var read = new NativeFunction(read, 'int', ['int', 'pointer', 'int']);
    var write = new NativeFunction(write, 'int', ['int', 'pointer', 'int']);
    var close = new NativeFunction(close, 'int', ['int']);


    var srcFd = open(Memory.allocUtf8String(srcPath), O_RDONLY, 0);
    if (srcFd < 0) {
        console.log("[X] Failed to open source file: " + srcPath);
        return;
    }
    console.log("[✔] Opened source file, fd = " + srcFd);

    var destFd = open(Memory.allocUtf8String(destPath), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (destFd < 0) {
        console.log("[X] Failed to open destination file: " + destPath);
        close(srcFd);
        return;
    }
    console.log("[✔] Opened destination file, fd = " + destFd);

    var buffer = Memory.alloc(1024);
    var bytesRead;
    while ((bytesRead = read(srcFd, buffer, 1024)) > 0) {
        write(destFd, buffer, bytesRead);
    }

    close(srcFd);
    close(destFd);
    console.log("[✔] File copied successfully: " + destPath);
}

function load_gum(){
    copyFile("/data/local/tmp/libgumTVM.so", "/data/data/" + package_name + "/libgumTVM.so")
    Module.load("/data/data/" + package_name + "/libgumTVM.so")
}

function hook_target_module(module_name, target_module){
    
    load_gum()
    
    var gum_trace_ptr = Module.findExportByName("libgumTVM.so", "gum_trace")
    console.log(gum_trace_ptr)
    var gum_trace = new NativeFunction(gum_trace_ptr, 'void', ['pointer', 'uint64', 'pointer']);
    gum_trace(Memory.allocUtf8String(Module_name), target_offset, Memory.allocUtf8String(trace_file_name))

} 

function main(){
    var Module_name_load
    var android_dlopen_ext_addr = Module.getExportByName("libandroid_runtime.so", "android_dlopen_ext");
    Interceptor.attach(android_dlopen_ext_addr, {
        onEnter:function(args){
            Module_name_load = args[0]
            console.log("android_dlopen_ext : ", Module_name_load.readCString())
        },onLeave:function(result){
            if(-1 != Module_name_load.readCString().indexOf(Module_name)){
                var target_module = Module.findBaseAddress(Module_name)
                console.log("target_module : ", target_module)
                hook_target_module(Module_name, target_module)
            }
        }
    })
}

setImmediate(main)