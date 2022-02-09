#pragma once

Entity CreateMusicCubeEntity(const DirectX::XMFLOAT3& position, const std::string& soundPath);
Entity CreateMusicPointLightEntity(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& light, const std::string& soundPath);
