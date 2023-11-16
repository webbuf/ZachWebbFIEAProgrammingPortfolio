#include "TextureManager.h"

unordered_map<string, sf::Texture> TextureManager::textures;

void TextureManager::LoadTexture(string textureName) {
	string path = "../../images/";
	path += textureName + ".png";
	textures[textureName].loadFromFile(path);
}

sf::Texture& TextureManager::GetTexture(string textureName) {
	if (textures.find(textureName) == textures.end()) {
		LoadTexture(textureName);
	}
	//loads file if it isn't found for safety
	//also allows for streamlined getting, doesn't load texture until it's needed
	return textures[textureName];
}

void TextureManager::Clear() {
	textures.clear();
}