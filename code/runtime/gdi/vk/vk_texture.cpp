internal vk_texture* VK_Texture_Manager_Allocate_Sync(resource_manager* ResourceManager) {
    gdi_texture TextureID = Resource_Manager_Allocate_ID(ResourceManager);
    if(!TextureID) {
        return NULL;
    }

    vk_texture* Texture = (vk_texture*)Resource_Manager_Get_Resource_Unsafe(ResourceManager, TextureID);
    Zero_Struct(Texture);
    Texture->ID = TextureID;
    return Texture;
}

internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(VK_Texture_Allocate_Callback) {
    return RESOURCE_RESULT_SUCCESS;
}

internal RESOURCE_FREE_CALLBACK_DEFINE(VK_Texture_Free_Callback) {
    return RESOURCE_RESULT_SUCCESS;
}
