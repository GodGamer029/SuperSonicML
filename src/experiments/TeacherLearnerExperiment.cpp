#include "TeacherLearnerExperiment.h"

//template<typename T>
TeacherLearnerExperiment/*<T, EnableIfBot<T>>*/::TeacherLearnerExperiment(std::shared_ptr<AtbaBot> bot) {
	this->teacherBot = bot;
	this->network = std::make_shared<Net>();
}

//template<typename T>
void TeacherLearnerExperiment/*<T, EnableIfBot<T>>*/::process(const BotInputData& input, ControllerInput& output) {
	ControllerInput teacherOutput;
	this->teacherBot->process(input, teacherOutput);
	auto expectedOutput = torch::tensor({ (float) teacherOutput.Throttle, (float) teacherOutput.Steer/*, teacherOutput.Handbrake ? 1.f : 0.f */});

	// Make state for learner
	auto ball = input.ball;
	auto car = input.car;
	auto targetLocal = dot(ball.pos - car.pos, car.orientation);
	auto dist = norm(ball.pos - car.pos);
	auto forwardSpeed = dot(car.vel, car.forward());

	// Normalize
	targetLocal /= 2000;
	targetLocal /= fmaxf(1.0f, norm(targetLocal) / 1);

	dist /= 6000;
	dist = clip(dist, -1, 1);

	forwardSpeed /= 2300;
	forwardSpeed = clip(forwardSpeed, -1, 1);

	static torch::optim::Adam optimizer(this->network->parameters(), torch::optim::AdamOptions(0.0005f /*learning rate*/));

	auto networkInput = torch::tensor({ targetLocal[0], targetLocal[1], targetLocal[2], dist, forwardSpeed });
	auto networkOutput = this->network->forward(networkInput);
	auto loss = torch::nn::functional::mse_loss(networkOutput, expectedOutput);

	optimizer.zero_grad();
	loss.backward();
	optimizer.step();

	output.Throttle = clip(networkOutput[0].item().toFloat(), -1.f, 1.f);
	output.Steer = clip(networkOutput[1].item().toFloat(), -1.f, 1.f);
	//output.Handbrake = networkOutput[2].item().toFloat() > 0.9f ? 1 : 0;
}
