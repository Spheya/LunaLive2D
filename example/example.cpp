#include <LunaLive2D.hpp>
#include <iostream>
#include <luna.hpp>
#include <imgui.h>
#include <fstream>
#include <sstream>

luna::Shader loadCustomShader() {
	std::fstream fragShader("assets/shaders/custom.fsh");
	std::fstream vertShader("assets/shaders/custom.vsh");
	std::stringstream fragBuffer;
	std::stringstream vertBuffer;
	fragBuffer << fragShader.rdbuf();
	vertBuffer << vertShader.rdbuf();
	luna::Shader shader(vertBuffer.str().c_str(), fragBuffer.str().c_str());
	shader.getProgram().setBlendMode(luna::BlendMode::On);
	shader.getProgram().setCullMode(luna::CullMode::Off);
	shader.getProgram().setDepthTestMode(luna::DepthTestMode::Off);
	return shader;
}

int main() {
	luna::setMessageCallback([](const char* message, const char* prefix, luna::MessageSeverity severity) {
		std::cout << "<" << prefix << "> " << message << std::endl;
	});
	csmSetLogFunction([](const char* message) {
		std::cout << "<Live2D> " << message << std::endl;
	});

	luna::initialize();
	luna::live2d::Model::loadShaders();

	luna::Window window;
	luna::Camera camera(&window);
	camera.setOrthographicSize(1.5f);

	luna::live2d::Model model;
	model.load("assets/models/hiyori/hiyori_free_t08.model3.json");
	model.setTransform(luna::Transform(glm::vec3(-0.4f, 0.0f, 0.0f)));
	
	luna::live2d::Model model2;
	model2.load("assets/models/hiyori/hiyori_free_t08.model3.json");
	model2.setTransform(luna::Transform(glm::vec3(0.4f, 0.0f, 0.0f)));

	luna::Shader shader = loadCustomShader();
	luna::Material mat(&shader);
	mat.setMainTexture(model2.getDrawables()[0].getTexture());
	for (size_t i = 0; i < model2.getDrawableCount(); ++i) {
		model2.getDrawables()[i].setMaterial(&mat);
	}

	while (!luna::isCloseRequested() && !window.isCloseRequested()) {
		luna::update();
		camera.updateAspect();

		luna::RenderTarget::clear(luna::Color(0.85f, 0.85f, 0.85f));

		model.update(luna::getDeltaTime());
		model.draw(camera);

		model2.update(luna::getDeltaTime());
		model2.draw(camera);

		mat.setValue("Time", luna::getTime());

		ImGui::Begin("Model Parameters");
		for (size_t i = 0; i < model.getParameterCount(); ++i) {
			auto& param = model.getParameters()[i];
			float value = param.getValue();
			ImGui::SliderFloat(param.getId(), &value, param.getMinValue(), param.getMaxValue());
			param.setValue(value);
			model2.getParameters()[i].setValue(value);
		}
		ImGui::End();

		window.update();
	}

	luna::terminate();
}