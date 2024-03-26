/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated July 28, 2023. Replaces all prior versions.
 *
 * Copyright (c) 2013-2023, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software or
 * otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THE
 * SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#define SPINE_SHORT_NAMES

#include "spine-sfml.h"

#ifndef SPINE_MESH_VERTEX_COUNT_MAX
#define SPINE_MESH_VERTEX_COUNT_MAX 1000
#endif

namespace spine {

SkeletonDrawable::SkeletonDrawable(SkeletonData *skeletonData, AnimationStateData *stateData) :
		timeScale(1),
		vertexArray(new sf::VertexArray(sf::Triangles, skeletonData->getBones().size() * 4)),
		vertexEffect(NULL), worldVertices(), clipper() {
	Bone::setYDown(true);
	worldVertices.ensureCapacity(SPINE_MESH_VERTEX_COUNT_MAX);
	skeleton = new(__FILE__, __LINE__) Skeleton(skeletonData);
	tempUvs.ensureCapacity(16);
	tempColors.ensureCapacity(16);

	ownsAnimationStateData = stateData == 0;
	if (ownsAnimationStateData) stateData = new(__FILE__, __LINE__) AnimationStateData(skeletonData);

	state = new(__FILE__, __LINE__) AnimationState(stateData);

	quadIndices.add(0);
	quadIndices.add(1);
	quadIndices.add(2);
	quadIndices.add(2);
	quadIndices.add(3);
	quadIndices.add(0);
}

SkeletonDrawable::~SkeletonDrawable() {
	delete vertexArray;
	if (ownsAnimationStateData) delete state->getData();
	delete state;
	delete skeleton;
}

void SkeletonDrawable::update(float deltaTime) {
	skeleton->update(deltaTime);
	state->update(deltaTime * timeScale);
	state->apply(*skeleton);
	skeleton->updateWorldTransform();
}

void SkeletonDrawable::draw(sf::RenderTarget &target, sf::RenderStates states) const {
	vertexArray->clear();
	states.texture = NULL;

	// Early out if skeleton is invisible
	if (skeleton->getColor().a == 0) return;

	if (vertexEffect != NULL) vertexEffect->begin(*skeleton);

	sf::Vertex vertex;
	sf::Texture *texture = NULL;
	for (unsigned i = 0; i < skeleton->getSlots().size(); ++i) {
		Slot &slot = *skeleton->getDrawOrder()[i];
		Attachment *attachment = slot.getAttachment();
		if (!attachment)
		{
			clipper.clipEnd(slot);
			continue;
		}

		// Early out if the slot color is 0 or the bone is not active
		if (slot.getColor().a == 0 || !slot.getBone().isActive()) {
			clipper.clipEnd(slot);
			continue;
		}

		const auto IsToBeLeftOut = [&list = m_leaveOutList](const char* szName)
			-> bool
			{
				for (const auto& name : list)
				{
					if (strstr(szName, name.c_str()) != nullptr)
					{
						return true;
					}
				}
				return false;
			};

		if (!bLeaveOutDisabled && IsToBeLeftOut(slot.getData().getName().buffer()))
		{
			clipper.clipEnd(slot);
			continue;
		}

		Vector<float> *vertices = &worldVertices;
		int verticesCount = 0;
		Vector<float> *uvs = NULL;
		Vector<unsigned short> *indices = NULL;
		int indicesCount = 0;
		Color *attachmentColor;

		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			RegionAttachment *regionAttachment = (RegionAttachment *) attachment;
			attachmentColor = &regionAttachment->getColor();

			// Early out if the slot color is 0
			if (attachmentColor->a == 0) {
				clipper.clipEnd(slot);
				continue;
			}

			worldVertices.setSize(8, 0);
			regionAttachment->computeWorldVertices(slot.getBone(), worldVertices, 0, 2);
			verticesCount = 4;
			uvs = &regionAttachment->getUVs();
			indices = &quadIndices;
			indicesCount = 6;
			texture = (sf::Texture *) ((AtlasRegion *) regionAttachment->getRendererObject())->page->getRendererObject();

		} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			MeshAttachment *mesh = (MeshAttachment *) attachment;
			attachmentColor = &mesh->getColor();

			// Early out if the slot color is 0
			if (attachmentColor->a == 0) {
				clipper.clipEnd(slot);
				continue;
			}

			worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
			texture = (sf::Texture *) ((AtlasRegion *) mesh->getRendererObject())->page->getRendererObject();
			mesh->computeWorldVertices(slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
			verticesCount = static_cast<int>(mesh->getWorldVerticesLength() >> 1);
			uvs = &mesh->getUVs();
			indices = &mesh->getTriangles();
			indicesCount = static_cast<int>(mesh->getTriangles().size());

		} else if (attachment->getRTTI().isExactly(ClippingAttachment::rtti)) {
			ClippingAttachment *clip = (ClippingAttachment *) slot.getAttachment();
			clipper.clipStart(slot, clip);
			continue;
		} else continue;

		sf::Uint8 r = static_cast<sf::Uint8>(skeleton->getColor().r * slot.getColor().r * attachmentColor->r * 255);
		sf::Uint8 g = static_cast<sf::Uint8>(skeleton->getColor().g * slot.getColor().g * attachmentColor->g * 255);
		sf::Uint8 b = static_cast<sf::Uint8>(skeleton->getColor().b * slot.getColor().b * attachmentColor->b * 255);
		sf::Uint8 a = static_cast<sf::Uint8>(skeleton->getColor().a * slot.getColor().a * attachmentColor->a * 255);
		vertex.color.r = r;
		vertex.color.g = g;
		vertex.color.b = b;
		vertex.color.a = a;

		Color light;
		light.r = r / 255.0f;
		light.g = g / 255.0f;
		light.b = b / 255.0f;
		light.a = a / 255.0f;

		usePremultipliedAlpha = r == 255 && g == 255 && b == 255 && a == 255;
		if (!usePremultipliedAlpha)
		{
			if (a > 96)
			{
				for (size_t ii = 0; ii < m_blendMultiplyList.size(); ++ii)
				{
					if (strstr(slot.getData().getName().buffer(), m_blendMultiplyList.at(ii).c_str()))
					{
						slot.getData().setBlendMode(spine::BlendMode::BlendMode_Multiply);
					}
				}
			}
		}
		sf::BlendMode blend;
		switch (slot.getData().getBlendMode())
		{
		case BlendMode_Additive:
			blend = sf::BlendMode(usePremultipliedAlpha ? sf::BlendMode::One: sf::BlendMode::SrcAlpha, sf::BlendMode::One);
			break;
		case BlendMode_Multiply:
			blend = sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha);
			break;
		case BlendMode_Screen:
			blend = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
			break;
		default:
			blend = sf::BlendMode(usePremultipliedAlpha ? sf::BlendMode::One: sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha);
			break;
		}

		if (states.texture == 0) states.texture = texture;

		if (states.blendMode != blend || states.texture != texture) {
			target.draw(*vertexArray, states);
			vertexArray->clear();
			states.blendMode = blend;
			states.texture = texture;
		}

		if (clipper.isClipping()) {
			clipper.clipTriangles(worldVertices, *indices, *uvs, 2);
			vertices = &clipper.getClippedVertices();
			verticesCount = static_cast<int>(clipper.getClippedVertices().size() >> 1);
			uvs = &clipper.getClippedUVs();
			indices = &clipper.getClippedTriangles();
			indicesCount = static_cast<int>(clipper.getClippedTriangles().size());
		}

		sf::Vector2u size = texture->getSize();

		if (vertexEffect != 0) {
			tempUvs.clear();
			tempColors.clear();
			for (int ii = 0; ii < verticesCount; ii++) {
				Color vertexColor = light;
				Color dark;
				dark.r = dark.g = dark.b = dark.a = 0;
				int index = ii << 1;
				float x = (*vertices)[index];
				float y = (*vertices)[index + 1LL];
				float u = (*uvs)[index];
				float v = (*uvs)[index + 1LL];
				vertexEffect->transform(x, y, u, v, vertexColor, dark);
				(*vertices)[index] = x;
				(*vertices)[index + 1LL] = y;
				tempUvs.add(u);
				tempUvs.add(v);
				tempColors.add(vertexColor);
			}

			for (int ii = 0; ii < indicesCount; ++ii) {
				int index = (*indices)[ii] << 1;
				vertex.position.x = (*vertices)[index];
				vertex.position.y = (*vertices)[index + 1LL];
				vertex.texCoords.x = (*uvs)[index] * size.x;
				vertex.texCoords.y = (*uvs)[index + 1LL] * size.y;
				Color vertexColor = tempColors[index >> 1];
				vertex.color.r = static_cast<sf::Uint8>(vertexColor.r * 255);
				vertex.color.g = static_cast<sf::Uint8>(vertexColor.g * 255);
				vertex.color.b = static_cast<sf::Uint8>(vertexColor.b * 255);
				vertex.color.a = static_cast<sf::Uint8>(vertexColor.a * 255);
				vertexArray->append(vertex);
			}
		} else {
			for (int ii = 0; ii < indicesCount; ++ii) {
				int index = (*indices)[ii] << 1;
				vertex.position.x = (*vertices)[index];
				vertex.position.y = (*vertices)[index + 1LL];
				vertex.texCoords.x = (*uvs)[index] * size.x;
				vertex.texCoords.y = (*uvs)[index + 1LL] * size.y;
				vertexArray->append(vertex);
			}
		}
		clipper.clipEnd(slot);
	}
	target.draw(*vertexArray, states);
	clipper.clipEnd();

	if (vertexEffect != 0) vertexEffect->end();
}

void SFMLTextureLoader::load(AtlasPage &page, const String &path) {
	sf::Texture *texture = new sf::Texture();
	if (!texture->loadFromFile(path.buffer())) return;

	if (page.magFilter == TextureFilter_Linear) texture->setSmooth(true);
	if (page.uWrap == TextureWrap_Repeat && page.vWrap == TextureWrap_Repeat) texture->setRepeated(true);

	page.setRendererObject(texture);
	/*In case atlas size does not coincide with that of png, overwriting will collapse the layout.*/
	//sf::Vector2u size = texture->getSize();
	//page.width = size.x;
	//page.height = size.y;
}

void SFMLTextureLoader::unload(void *texture) {
	delete (sf::Texture *) texture;
}

SpineExtension *getDefaultExtension() {
	return new DefaultSpineExtension();
}
} // namespace spine
