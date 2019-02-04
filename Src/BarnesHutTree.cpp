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
    // ������� ������� � ������
    if (curDepth > 40) {
        return;
    }

    if (isLeaf_) {
        // ���� ���� - ����
        if (!particle_) {
            // � ������, �� ��������� � ���� �������
            particle_ = &p;
            return;
        } else {
            // ���� ���� �������� �� ���������� ���������� �����			
            isLeaf_ = false;

            if (!subs_[0]) {
                // ������� �������� � �������� ������
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
                // ����� ���������� ��
                subs_[0]->isLeaf_ = true;
                subs_[1]->isLeaf_ = true;
                subs_[2]->isLeaf_ = true;
                subs_[3]->isLeaf_ = true;

                subs_[0]->particle_ = 0;
                subs_[1]->particle_ = 0;
                subs_[2]->particle_ = 0;
                subs_[3]->particle_ = 0;
            }

            // ����� ��������� � ������ ������� ������� ������� ���� � ������� ����
            for (int i = 0; i < 4; i++) {
                if (subs_[i]->contains(*particle_)) {
                    subs_[i]->insert(*particle_);
                    break;
                }
            }

            // � ����� �������
            for (int i = 0; i < 4; i++) {
                if (subs_[i]->contains(p)) {
                    curDepth++;
                    subs_[i]->insert(p);
                    break;
                }
            }

            // ��������� ����� ����
            totalMass_ = particle_->m_mass + p.m_mass;

            // ����� �������
            massCenter = p.m_pos.scaleR(p.m_mass);
            massCenter.addScaled(particle_->m_pos, particle_->m_mass);
            massCenter *= 1.0f / totalMass_;
        }
    } else {
        // ���� ��� ���������� ����

        // ��������� ��������� ����� ����������� � ��� ����� ����� �������
        float total = totalMass_ + p.m_mass;

        // ����� ��������� ����� ����
        massCenter *= totalMass_;
        massCenter.addScaled(p.m_pos, p.m_mass);
        massCenter *= 1.0f / total;
        totalMass_ = total;

        // ���������� ��������� � ������ ������� �������
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
    // ������ ���� ����������� �� �������

    float softFactor2 = 1.0e-0f;

    if (isLeaf_ && particle_) {
        if (particle_ != &p) {
            // ���� ��� ���� � � ��� ���������� ������� �������� �� �������
            // ��������� ���� ����������� ����� ���������
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
        // ���� ��� ���������� ����

        // ������� ���������� �� ������� �� ������ ���� ����� ����
        lpVec3 force = massCenter;
        force -= p.m_pos;

        float r = force.norm();

        // ������� ����������� ������� ���� � ����������
        float theta = length_ / r;

        if (theta < 0.7f) {
            float r2 = r * r + softFactor2;
            r2 *= r2 * r2;
            r2 = sqrtf(r2);

            // ���� ������� ������� �� ���� �� ����������� ����������
            // ������������� ���� ��� ���� ������� � ��������� ��������� ������ � ������� ����
            // � ��������� ����
            force *= totalMass_ * p.m_mass / r2;
            p.m_forceAccum += force;
        } else {
            // ���� ������� ������ � ���� ���������� ������� ���� � ���������
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