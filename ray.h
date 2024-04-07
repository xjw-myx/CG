#ifndef RAY_H
#define RAY_H

#include <QVector3D>
#include <QColor>

#define TOTALDEPTH 2 // 光线递归深度
#define INFINITY 1000000.0f
#define SMALL 0.0001f
enum INTERSECTION_TYPE {INTERSECTED_IN = -1,MISS = 0,INTERSECTED = 1};

class Ray
{
public:
    Ray();
    Ray(const QVector3D& org, const QVector3D& drct);

    const QVector3D& get_origin() const;
    const QVector3D& get_direction() const;
    void set_origin(const QVector3D& org);
    void set_direction(const QVector3D& drct);
    QVector3D get_point(float d) const;

private:
    QVector3D m_origin;
    QVector3D m_direction;
};

class Object
{
public:
    Object();
    float get_spec();
    float get_refl();
    float get_diffuse();
    void reset(float spec, float refl);
    virtual QVector3D get_color(QVector3D position) = 0;
    virtual QVector3D get_normal(QVector3D point) = 0;                  // 获取物体表面一点的法线
    virtual INTERSECTION_TYPE is_intersected(Ray ray,float& dst) = 0;   // 判断光线是否与物体相交
    int style;
protected:
    float m_spec;                                                       // 镜面反射强度
    float m_refl;                                                       // 环境反射强度
};

class PointLight
{
public:
    PointLight();
    QVector3D get_position() const;
    QColor get_color() const;
    void reset(const QVector3D& position, const QColor& color);

private:
    QVector3D m_position;
    QColor m_color;
};

class Help
{
public:
    Help();
    ~Help();

    void init();
    int get_object_count();
    int get_light_count();
    void set_ambient_light(QColor amb);
    QColor get_ambient_light();
    Object* get_object(int idx);
    PointLight get_light(int idx);
    Object** m_obj;
    PointLight* m_light;
    int loadHelp(char* filename);
    void parse_check(char* expected, char* found);
    void parse_doubles(FILE* file, char* check, double p[3]);
    void parse_rad(FILE* file, double* r);
    void parse_shi(FILE* file, double* shi);

private:
    int m_object_count;
    int m_light_count;
    QColor ambient_light;
};


#endif // RAY_H
