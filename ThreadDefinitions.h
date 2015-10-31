#pragma once
#include <utility/Handle.h>

enum class ThreadType {
	Any, 		// Will run any task pushed to the common queue
	Dedicated, 	// Will only run tasks dedicated to the thread
	Named, 		// Tasks can be assigned to the thread but it will also run tasks from the common queue
};

struct ThreadIdentifier_Tag{};

typedef Handle<ThreadIdentifier_Tag, int, -1> ThreadIdentifier;

