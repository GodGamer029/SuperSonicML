#pragma once

#include "Experiment.h"
#include <bots/BotClass.h>
#include <bots/AtbaBot.h>
#include <type_traits>
#include <memory>
#include <torch/torch.h>
#include <vector>
#include <random>
#include <deque>
#include <GameData.h>

// https://pytorch.org/cppdocs/frontend.html
struct Net : torch::nn::Module {
	torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};

	Net() {
		fc1 = register_module("fc1", torch::nn::Linear(10, 40));
		fc2 = register_module("fc2", torch::nn::Linear(40, 30));
		fc3 = register_module("fc3", torch::nn::Linear(30, 3));
		//fc4 = register_module("fc4", torch::nn::Linear(20, 3));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x) {
		// Use one of many tensor manipulation functions.
		x = torch::leaky_relu(fc1->forward(x));
		x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
		x = torch::leaky_relu(fc2->forward(x));
		x = torch::dropout(x, /*p=*/0.3, /*train=*/is_training());
		x = torch::nn::functional::softsign(fc3->forward(x)).clamp(-1, 1);
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

	std::deque<std::tuple<float/*loss*/, std::vector<float>/*input*/, std::vector<float>/*expected output*/>> replayMemory;
	double totalLossInReplayMemory = 0;

	void processGroundBased(const BotInputData&, ControllerInput& output);
	void processAirBased(const BotInputData&, ControllerInput& output);

public:
	TeacherLearnerExperiment(std::shared_ptr<AtbaBot> bot);

	void process(const BotInputData&, ControllerInput& output);
};
