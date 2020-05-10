#pragma once

#include "Experiment.h"
#include <bots/BotClass.h>
#include <bots/AtbaBot.h>
#include <type_traits>
#include <memory>
#include <torch/torch.h>

// https://pytorch.org/cppdocs/frontend.html
struct Net : torch::nn::Module {
	torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};

	Net() {
		fc1 = register_module("fc1", torch::nn::Linear(5, 10));
		fc2 = register_module("fc2", torch::nn::Linear(10, 8));
		fc3 = register_module("fc3", torch::nn::Linear(8, 2));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x) {
		// Use one of many tensor manipulation functions.
		x = torch::relu(fc1->forward(x));
		//x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
		x = torch::relu(fc2->forward(x));
		x = fc3->forward(x).clamp(-1, 1); // no activation
		return x;
	}

};

// All this template stuff works, but autocomplete doesnt
#ifdef USE_TEMPLATES
template<typename T>
using EnableIfBot = typename std::enable_if<std::is_base_of<BotClass, T>::value>::type;

template <typename T, typename Enable = void>
class TeacherLearnerExperiment; // Idk why this is here, but it doesn't compile without it

//template<typename T>
#endif

class TeacherLearnerExperiment/*<T, EnableIfBot<T>>*/ : Experiment {
private:
	std::shared_ptr<AtbaBot> teacherBot;
	std::shared_ptr<Net> network;
public:
	TeacherLearnerExperiment(std::shared_ptr<AtbaBot> bot);

	void process(const BotInputData&, ControllerInput& output);
};
