
#include "stdafx.h"
/**
*  Native function that is callable from PHP
*/
Php::Value run_test() {
	// create the anonymous function
	Php::Function multiply_by_two([](Php::Parameters &params) -> Php::Value {

		// make sure the function was really called with at least one parameter
		if (params.empty()) return nullptr;

		// one parameter is passed to the function
		Php::Value param = params[0];

		// multiple the parameter by two
		return param * 2;
	});

	// the function now is callable
	Php::Value four = multiply_by_two(2);

	// a Php::Function object is a derived Php::Value, and its value can 
	// also be stored in a normal Php::Value object, it will then still 
	// be a callback function then
	Php::Value value = multiply_by_two;

	auto type = value.type();

	// the value object now also holds the function
	Php::Value six = value(3);

	// create an array
	Php::Value array;
	array[0] = 1;
	array[1] = 2;
	array[2] = 3;
	array[3] = 4;
	array[4] = six;

	// call the user-space function
	Php::Value result = Php::call("my_array_map", array, multiply_by_two);

	type = result.type();

	// @todo do something with the result variable (which now holds
	// an array with values 2, 4, 6 and 8).
	return result;
}

/**
*  Switch to C context, because the Zend engine expects the get_module()
*  to have a C style function signature
*/
extern "C" {
	/**
	*  Startup function that is automatically called by the Zend engine
	*  when PHP starts, and that should return the extension details
	*  @return void*
	*/
	SPA_PHP_EXPORT void *get_module() {
		// the extension object
		static Php::Extension extension("my_extension", "1.0");

		// add the example function so that it can be called from PHP scripts
		extension.add("run_test", run_test);

		// return the extension details
		return extension;
	}
}
