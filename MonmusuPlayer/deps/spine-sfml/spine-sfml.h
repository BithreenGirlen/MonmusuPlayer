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

#ifndef SPINE_SFML_H_
#define SPINE_SFML_H_

#include <spine/spine.h>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>


namespace spine {

class SkeletonDrawable : public sf::Drawable {
public:
	Skeleton *skeleton;
	AnimationState *state;
	float timeScale;
	sf::VertexArray *vertexArray;
	VertexEffect* vertexEffect;

	SkeletonDrawable(SkeletonData *skeleton, AnimationStateData *stateData = 0);

	~SkeletonDrawable();

	void update(float deltaTime);

	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

	void SetBlendMultiplyList(const std::vector<std::string>& list)
	{
		m_blendMultiplyList = list;
	}
	void SetLeaveOutList(const std::vector<std::string>& list)
	{
		m_leaveOutList = list;
	}
	void SwitchHiddenness()const { bLeaveOutDisabled ^= true; }

private:
	mutable bool ownsAnimationStateData;
	mutable Vector<float> worldVertices;
	mutable Vector<float> tempUvs;
	mutable Vector<Color> tempColors;
	mutable Vector<unsigned short> quadIndices;
	mutable SkeletonClipping clipper;
	mutable bool usePremultipliedAlpha = false;
	std::vector<std::string> m_blendMultiplyList;
	mutable bool bLeaveOutDisabled = false;
	std::vector<std::string> m_leaveOutList;
};

class SFMLTextureLoader : public TextureLoader {
public:
	virtual void load(AtlasPage &page, const String &path);

	virtual void unload(void *texture);

	String toString() const;
};

} /* namespace spine */
#endif /* SPINE_SFML_H_ */