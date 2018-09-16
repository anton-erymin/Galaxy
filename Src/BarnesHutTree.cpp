#include "BarnesHutTree.h"

#include "gl\glut.h"

#include "GalaxySystem.h"
#include "constants.h"

int curDepth;

BarnesHutTree::BarnesHutTree(const lpVec3 &point, float length)
    : point_(point),
      length_(length),
      isLeaf_(true),
      particle_(nullptr),
      totalMass_(0.0f) {
    oppositePoint_ = point_ + lpVec3{length_};
}

void BarnesHutTree::reset() {
    isLeaf_   = true;
    particle_ = nullptr;
}

void BarnesHutTree::insert(const GalaxyParticle &p) {
    // Вставка частицы в дерево
    if (curDepth > 40) {
        return;
    }

    if (isLeaf_) {
        // Если узел - лист
        if (!particle_) {
            // И пустой, то вставляем в него частицу
            particle_ = &p;
            return;
        } else {
            // Если лист непустой он становится внутренним узлом			
            isLeaf_ = false;

            if (!subs_[0]) {
                // Размеры потомков в половину меньше
                float nl = 0.5f * length_;

                float x = point_.m_x + nl;
                float y = point_.m_y + nl;

                lpVec3 np1(x, point_.m_y, 0.0f);
                lpVec3 np2(x, y, 0.0f);
                lpVec3 np3(point_.m_x, y, 0.0f);

                subs_[0] = std::make_unique<BarnesHutTree>(point_, nl);
                subs_[1] = std::make_unique<BarnesHutTree>(np1, nl);
                subs_[2] = std::make_unique<BarnesHutTree>(np2, nl);
                subs_[3] = std::make_unique<BarnesHutTree>(np3, nl);
            } else {
                // Иначе сбрасываем их
                subs_[0]->isLeaf_ = true;
                subs_[1]->isLeaf_ = true;
                subs_[2]->isLeaf_ = true;
                subs_[3]->isLeaf_ = true;

                subs_[0]->particle_ = 0;
                subs_[1]->particle_ = 0;
                subs_[2]->particle_ = 0;
                subs_[3]->particle_ = 0;
            }

            // Далее вставляем в нужный потомок частицу которая была в текущем узле
            for (int i = 0; i < 4; i++) {
                if (subs_[i]->contains(*particle_)) {
                    subs_[i]->insert(*particle_);
                    break;
                }
            }

            // И новую частицу
            for (int i = 0; i < 4; i++) {
                if (subs_[i]->contains(p)) {
                    curDepth++;
                    subs_[i]->insert(p);
                    break;
                }
            }

            // Суммарная масса узла
            totalMass_ = particle_->m_mass + p.m_mass;

            // Центр тяжести
            massCenter = p.m_pos.scaleR(p.m_mass);
            massCenter.addScaled(particle_->m_pos, particle_->m_mass);
            massCenter *= 1.0f / totalMass_;
        }
    } else {
        // Если это внутренний узел

        // Обновляем суммарную массу добавлением к ней массы новой частицы
        float total = totalMass_ + p.m_mass;

        // Также обновляем центр масс
        massCenter *= totalMass_;
        massCenter.addScaled(p.m_pos, p.m_mass);
        massCenter *= 1.0f / total;
        totalMass_ = total;

        // Рекурсивно вставляем в нужный потомок частицу
        for (int i = 0; i < 4; i++) {
            if (subs_[i]->contains(p)) {
                curDepth++;
                subs_[i]->insert(p);
                break;
            }
        }
    }
}


bool BarnesHutTree::contains(const GalaxyParticle &p) const {
    lpVec3 v = p.m_pos;
    if (v.m_x >= point_.m_x && v.m_x <= oppositePoint_.m_x &&
        v.m_y >= point_.m_y && v.m_y <= oppositePoint_.m_y)
        return true;

    return false;
}


void BarnesHutTree::calcForce(GalaxyParticle &p) const {
    // Расчет силы действующей на частицу

    float softFactor2 = 1.0e-0f;

    if (isLeaf_ && particle_) {
        if (particle_ != &p) {
            // Если это лист и в нем содержится частица отличная от текущей
            // Вычисляем силу действующую между частицами
            lpVec3 force = particle_->m_pos;
            force -= p.m_pos;

            float r2 = force.normSq();
            r2 += softFactor2;
            r2 *= r2 * r2;
            r2 = sqrtf(r2);

            force *= particle_->m_mass * p.m_mass / r2;
            p.m_forceAccum += force;

            return;
        }
    }

    if (!isLeaf_) {
        // Если это внутренний узел

        // Находим расстояние от частицы до центра масс этого узла
        lpVec3 force = massCenter;
        force -= p.m_pos;

        float r = force.norm();

        // Находим соотношение размера узла к расстоянию
        float theta = length_ / r;

        if (theta < 0.7f) {
            float r2 = r * r + softFactor2;
            r2 *= r2 * r2;
            r2 = sqrtf(r2);

            // Если частица удалена от узла на достаточное расстояние
            // Рассматриваем узел как одну частицу с известной суммарной массой и центром масс
            // И Вычисляем силу
            force *= totalMass_ * p.m_mass / r2;
            p.m_forceAccum += force;
        } else {
            // Если частица близко к узлу рекурсивно считаем силу с потомками
            for (int i = 0; i < 4; i++) {
                subs_[i]->calcForce(p);
            }

        }

    }
}

void drawTree(const BarnesHutTree &node) {
    lpVec3 p = node.point();
    float l  = node.length();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 1.0f, 0.0f);

    glVertex3f(p.m_x, p.m_y, 0.0f);
    glVertex3f(p.m_x + l, p.m_y, 0.0f);
    glVertex3f(p.m_x + l, p.m_y + l, 0.0f);
    glVertex3f(p.m_x, p.m_y + l, 0.0f);
    glVertex3f(p.m_x, p.m_y, 0.0f);
    glEnd();

    if (!node.isLeaf()) {
        for (size_t i = 0; i < 4; i++) {
            drawTree(node[i]);
        }
    }
}