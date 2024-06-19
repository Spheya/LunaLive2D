#include <LunaLive2D.hpp>
#include <iostream>
#include <luna.hpp>
#include <imgui.h>
#include <fstream>
#include <sstream>

luna::Shader loadCustomShaders() {
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
		std::cout << "<Live2D> " << message;
	});

	// setup luna
	luna::initialize();
	luna::live2d::Model::loadShaders();
	luna::Window window("Live2D Example", 1200, 800);
	luna::Camera camera(&window);
	camera.setOrthographicSize(1.5f);

	// load models
	luna::live2d::Model model("assets/models/hiyori/hiyori_free_t08.model3.json");
	luna::live2d::ModelInstance model1(&model);
	luna::live2d::ModelInstance model2(&model);
	luna::live2d::Renderer modelRenderer(&model1);
	luna::live2d::Renderer model2Renderer(&model2);
	model1.setTransform(luna::Transform(glm::vec3(-0.4f, 0.0f, 0.0f)));
	model2.setTransform(luna::Transform(glm::vec3(0.4f, 0.0f, 0.0f)));

	// load/apply custom shaders
	luna::Shader shader = loadCustomShaders();
	luna::Material mat(&shader);
	mat.setMainTexture(model2.getDrawables()[0].getTexture());
	for (size_t i = 0; i < model2.getDrawableCount(); ++i) {
		model2.getDrawables()[i].setMaterial(&mat);
	}

	// game loop
	while (!luna::isCloseRequested() && !window.isCloseRequested()) {
		luna::update();
		camera.updateAspect();
		modelRenderer.beginFrame();
		model2Renderer.beginFrame();
		luna::RenderTarget::clear(luna::Color(0.85f, 0.85f, 0.85f));

		mat.setValue("Time", luna::getTime());

		model1.update(luna::getDeltaTime());
		model2.update(luna::getDeltaTime());

		ImGui::Begin("Model Parameters");
		for (size_t i = 0; i < model1.getParameterCount(); ++i) {
			auto& param = model1.getParameters()[i];
			float value = param.getValue();
			ImGui::SliderFloat(param.getId(), &value, param.getMinValue(), param.getMaxValue());
			param.setValue(value);
			model2.getParameters()[i].setValue(value);
		}
		ImGui::End();

		modelRenderer.endFrame();
		model2Renderer.endFrame();
		modelRenderer.render(camera);
		model2Renderer.render(camera);
		window.update();
	}

	luna::live2d::Model::unloadShaders();
	luna::terminate();
}