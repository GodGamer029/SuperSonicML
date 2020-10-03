#pragma once

#include "Experiment.h"
#include <bots/BotClass.h>
#include <bots/AtbaBot.h>
#include <bots/AerialAtbaBot.h>
#include <type_traits>
#include <memory>
#include <torch/torch.h>
#include <vector>
#include <random>
#include <deque>
#include <GameData.h>
#include <array>

// https://pytorch.org/cppdocs/frontend.html
struct Net : torch::nn::Module {
	torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr}, fc4{nullptr};

	Net() {
		fc1 = register_module("fc1", torch::nn::Linear(8 * 3 + 3, 80));
		fc2 = register_module("fc2", torch::nn::Linear(80, 60));
		fc3 = register_module("fc3", torch::nn::Linear(60, 30));
		fc4 = register_module("fc4", torch::nn::Linear(30, 8));
	}

	torch::Tensor forward(torch::Tensor x) {
		x = torch::gelu(fc1->forward(x));
		x = torch::dropout(x, /*p=*/0.3, /*train=*/is_training());
		x = torch::gelu(fc2->forward(x));
		x = torch::dropout(x, /*p=*/0.3, /*train=*/is_training());
		x = torch::gelu(fc3->forward(x));
		x = torch::dropout(x, /*p=*/0.3, /*train=*/is_training());
		x = torch::tanh(fc4->forward(x));
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
	static constexpr auto REPLAYMEMORY_MIN = 120 * 3; // 3 seconds
	static constexpr auto REPLAYMEMORY_MAX = 120 * 60 * 3; // 3 minutes of memory
	static constexpr auto INPUT_SIZE = 8 * 3 + 3;
	static constexpr auto OUTPUT_SIZE = 8;

	std::shared_ptr<AerialAtbaBot> teacherBot;
	std::shared_ptr<Net> network;

	std::deque<std::tuple<float/*loss*/, std::array<float, INPUT_SIZE>/*input*/, std::array<float, OUTPUT_SIZE>/*expected output*/, float/*multiplier*/>> replayMemory;
	double totalLossInReplayMemory = 0;

	void processGroundBased(const BotInputData&, ControllerInput& output);
	void processAirBased(const BotInputData&, ControllerInput& output);

public:
	TeacherLearnerExperiment(std::shared_ptr<AerialAtbaBot> bot);

	void process(const BotInputData&, ControllerInput& output);
};
