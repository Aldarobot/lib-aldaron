//
#[cfg(target_os = "android")]
#[macro_use] 
extern crate android_glue;

#[cfg(target_os = "android")]
android_start!(main);

//
extern crate jl_lib;

fn main() {
	println!(":jl - ya see!\n");
	jl_lib::jl_init();
	jl_lib::jl_loop();
}
