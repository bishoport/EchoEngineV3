#include "IBL.h"
#include "../../managers/ShaderManager.h"
#include "../../managers/TextureManager.h"

namespace libCore
{
	void IBL::prepareIBL(int screenWidth, int screenHeight, bool dynamic)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		m_dynamic = dynamic;

		if (m_dynamic == false)
		{
			configureStaticIBL();
		}
		else
		{
			configureDynamicIBL();
		}
	}

	

	void IBL::configureStaticIBL()
	{
		m_hdrTexture = TextureManager::getInstance().loadHDR("assets/HDR/newport_loft.hdr");

		glGenFramebuffers(1, &IBL_FBO);
		glGenRenderbuffers(1, &IBL_RBO);

		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_screenWidth, m_screenWidth);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, IBL_RBO);


		// pbr: setup cubemap to render to and attach to framebuffer
		// ---------------------------------------------------------
		glGenTextures(1, &envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_screenWidth, m_screenWidth, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
		// ----------------------------------------------------------------------------------------------
		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};



		// pbr: convert HDR equirectangular environment map to cubemap equivalent
		// ----------------------------------------------------------------------
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
		libCore::ShaderManager::Get("equirectangularToCubemap")->use();
		libCore::ShaderManager::Get("equirectangularToCubemap")->setInt("equirectangularDayLightMap", 0);
		libCore::ShaderManager::Get("equirectangularToCubemap")->setFloat("mixFactor", 0.0);
		libCore::ShaderManager::Get("equirectangularToCubemap")->setMat4("projection", captureProjection);


		glViewport(0, 0, m_screenWidth, m_screenHeight); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			libCore::ShaderManager::Get("equirectangularToCubemap")->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
		// --------------------------------------------------------------------------------
		glGenTextures(1, &irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);


		// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
		// -----------------------------------------------------------------------------
		libCore::ShaderManager::Get("irradiance")->use();
		libCore::ShaderManager::Get("irradiance")->setInt("environmentMap", 0);
		libCore::ShaderManager::Get("irradiance")->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glViewport(0, 0, m_screenWidth, m_screenHeight); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			libCore::ShaderManager::Get("irradiance")->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderCube();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
		// --------------------------------------------------------------------------------
		glGenTextures(1, &prefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
		// ----------------------------------------------------------------------------------------------------
		libCore::ShaderManager::Get("prefilter")->use();
		libCore::ShaderManager::Get("prefilter")->setInt("environmentMap", 10);
		libCore::ShaderManager::Get("prefilter")->setMat4("projection", captureProjection);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipWidth = static_cast<unsigned int>(256 * std::pow(0.5, mip));
			unsigned int mipHeight = static_cast<unsigned int>(256 * std::pow(0.5, mip));
			glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			libCore::ShaderManager::Get("prefilter")->setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				libCore::ShaderManager::Get("prefilter")->setMat4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				renderCube();
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);





		// pbr: generate a 2D LUT from the BRDF equations used.
		// ----------------------------------------------------
		glGenTextures(1, &brdfLUTTexture);

		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_screenWidth, m_screenHeight, 0, GL_RG, GL_FLOAT, 0);
		// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_screenWidth, m_screenHeight);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, m_screenWidth, m_screenHeight);
		libCore::ShaderManager::Get("brdf")->use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}



	void IBL::configureDynamicIBL() {
		//// Generar framebuffers y renderbuffers
		//glGenFramebuffers(1, &IBL_FBO);
		//glGenRenderbuffers(1, &IBL_RBO);

		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_screenWidth, m_screenWidth);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, IBL_RBO);

		//// Configurar el cubemap de entorno
		//glGenTextures(1, &envCubemap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//for (unsigned int i = 0; i < 6; ++i) {
		//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_screenWidth, m_screenWidth, 0, GL_RGB, GL_FLOAT, nullptr);
		//}
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
		//// ----------------------------------------------------------------------------------------------
		//glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		//glm::mat4 captureViews[] =
		//{
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		//};



		//// pbr: convert HDR equirectangular environment map to cubemap equivalent
		//// ----------------------------------------------------------------------
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
		//libCore::ShaderManager::Get("equirectangularToCubemap")->use();
		//libCore::ShaderManager::Get("equirectangularToCubemap")->setInt("equirectangularDayLightMap", 0);
		//libCore::ShaderManager::Get("equirectangularToCubemap")->setFloat("mixFactor", 0.0);
		//libCore::ShaderManager::Get("equirectangularToCubemap")->setMat4("projection", captureProjection);


		//glViewport(0, 0, m_screenWidth, m_screenHeight); // don't forget to configure the viewport to the capture dimensions.
		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//for (unsigned int i = 0; i < 6; ++i)
		//{
		//	libCore::ShaderManager::Get("equirectangularToCubemap")->setMat4("view", captureViews[i]);
		//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//	renderCube();
		//}
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		//// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
		//// --------------------------------------------------------------------------------
		//glGenTextures(1, &irradianceMap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		//for (unsigned int i = 0; i < 6; ++i)
		//{
		//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		//}
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);


		//// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
		//// -----------------------------------------------------------------------------
		//libCore::ShaderManager::Get("irradiance")->use();
		//libCore::ShaderManager::Get("irradiance")->setInt("environmentMap", 0);
		//libCore::ShaderManager::Get("irradiance")->setMat4("projection", captureProjection);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		//glViewport(0, 0, m_screenWidth, m_screenHeight); // don't forget to configure the viewport to the capture dimensions.
		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//for (unsigned int i = 0; i < 6; ++i)
		//{
		//	libCore::ShaderManager::Get("irradiance")->setMat4("view", captureViews[i]);
		//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//	renderCube();
		//}
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
		//// --------------------------------------------------------------------------------
		//glGenTextures(1, &prefilterMap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		//for (unsigned int i = 0; i < 6; ++i)
		//{
		//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
		//}
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
		//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		//// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
		//// ----------------------------------------------------------------------------------------------------
		//libCore::ShaderManager::Get("prefilter")->use();
		//libCore::ShaderManager::Get("prefilter")->setInt("environmentMap", 10);
		//libCore::ShaderManager::Get("prefilter")->setMat4("projection", captureProjection);
		//glActiveTexture(GL_TEXTURE10);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//unsigned int maxMipLevels = 5;
		//for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		//{
		//	// reisze framebuffer according to mip-level size.
		//	unsigned int mipWidth = static_cast<unsigned int>(256 * std::pow(0.5, mip));
		//	unsigned int mipHeight = static_cast<unsigned int>(256 * std::pow(0.5, mip));
		//	glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		//	glViewport(0, 0, mipWidth, mipHeight);

		//	float roughness = (float)mip / (float)(maxMipLevels - 1);
		//	libCore::ShaderManager::Get("prefilter")->setFloat("roughness", roughness);
		//	for (unsigned int i = 0; i < 6; ++i)
		//	{
		//		libCore::ShaderManager::Get("prefilter")->setMat4("view", captureViews[i]);
		//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

		//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//		renderCube();
		//	}
		//}
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//// Crear un LUT 2D para BRDF
		//glGenTextures(1, &brdfLUTTexture);
		//glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_screenWidth, m_screenHeight, 0, GL_RG, GL_FLOAT, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, IBL_RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_screenWidth, m_screenHeight);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		//glViewport(0, 0, m_screenWidth, m_screenHeight);
		//libCore::ShaderManager::Get("brdf")->use();
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//renderQuad();

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void IBL::UpdateIBL(const glm::vec3& cameraPosition, const std::vector<Ref<libCore::Model>>& modelsInScene, float nearPlane, float farPlane)
	{
		//if (!m_dynamic) return;

		//// Configurar las matrices de proyección y vista para capturar el entorno
		//glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

		//glm::mat4 captureViews[] =
		//{
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(cameraPosition, cameraPosition + glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		//};

		//glViewport(0, 0, m_screenWidth, m_screenHeight);
		//glBindFramebuffer(GL_FRAMEBUFFER, IBL_FBO);

		//libCore::ShaderManager::Get("captureIBL")->use();
		//libCore::ShaderManager::Get("captureIBL")->setMat4("projection", captureProjection);
		//libCore::ShaderManager::Get("captureIBL")->setVec3("viewPos", cameraPosition);
		//libCore::ShaderManager::Get("captureIBL")->setVec3("lightPos", glm::vec3(0.0f, 10.0f, 0.0f)); // Posición de la luz
		//libCore::ShaderManager::Get("captureIBL")->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // Color de la luz

		//// Establecer propiedades del material desde las variables miembro
		//libCore::ShaderManager::Get("captureIBL")->setFloat("ambientStrength", ambientStrength);
		//libCore::ShaderManager::Get("captureIBL")->setFloat("specularStrength", specularStrength);
		//libCore::ShaderManager::Get("captureIBL")->setFloat("shininess", shininess);

		//for (unsigned int i = 0; i < 6; ++i) {
		//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//	libCore::ShaderManager::Get("captureIBL")->setMat4("view", captureViews[i]);

		//	for (const auto& model : modelsInScene) {
		//		model->Draw("captureIBL"); // Renderizar cada modelo usando el shader de captura
		//	}
		//}

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}


	void IBL::renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	void IBL::renderCube()
	{
		// initialize (if necessary)
		if (cubeVAO == 0)
		{
			float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				 // bottom face
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 // top face
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);
			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			// link vertex attributes
			glBindVertexArray(cubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		// render Cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}
