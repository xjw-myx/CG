#include "ray.h"

Ray::Ray()
{
    m_origin = QVector3D(0, 0, 0);
    m_direction = QVector3D(0, 0, 1);
}

Ray::Ray(const QVector3D& org, const QVector3D& drct)
{
    m_origin = org;
    m_direction = drct;
}

const QVector3D& Ray::get_origin() const
{
    return m_origin;
}

const QVector3D& Ray::get_direction() const
{
    return m_direction;
}

void Ray::set_origin(const QVector3D& org)
{
    m_origin = org;
}

void Ray::set_direction(const QVector3D& drct)
{
    m_direction = drct;
}

QVector3D Ray::get_point(float d) const
{
    return m_origin + d * m_direction;
}



Object::Object()
{
    m_spec = 1.0f;
    m_refl = 1.0f;
}

float Object::get_spec()
{
    return m_spec;
}

float Object::get_refl()
{
    return m_refl;
}

float Object::get_diffuse()
{
    return 1.0f - m_spec;
}

void Object::reset(float spec, float refl)
{
    m_spec = spec;
    m_refl = refl;
}



PointLight::PointLight()
{
    m_position = QVector3D(0, 0, 0);
    m_color = QColor(255, 255, 255); // 默认为白色
}

QVector3D PointLight::get_position() const
{
    return m_position;
}

QColor PointLight::get_color() const
{
    return m_color;
}

void PointLight::reset(const QVector3D& position, const QColor& color)
{
    m_position = position;
    m_color = color;
}



Help::Help()
{
}

Help::~Help()
{
    delete m_obj;
    delete m_light;
}

void Help::init()
{

}

void Help::set_ambient_light(QColor amb)
{
    ambient_light = amb;
}

QColor Help::get_ambient_light()
{
    return ambient_light;
}

int Help::get_object_count()
{
    return m_object_count;
}

int Help::get_light_count()
{
    return m_light_count;
}

Object* Help::get_object(int idx)
{
    return m_obj[idx];
}

PointLight Help::get_light(int idx)
{
    return m_light[idx];
}

int Help::loadHelp(char *filename)
{
    m_light = new PointLight[100];
    m_obj = new Object*[1500];
    FILE *file = fopen(filename, "r");
    int number_of_objects;
    char type[50];
    int i;
    fscanf(file, "%i", &number_of_objects);

    printf("number of objects: %i\n", number_of_objects);
    char str[200];

    double ambient_light[3];
    parse_doubles(file, "amb:", ambient_light);
//	ambient_light = QColor(ambient_light[0], ambient_light[1], ambient_light[2]);

    for (i = 0; i < number_of_objects; i++)
    {
        fscanf(file, "%s\n", type);
        if (stricmp(type, "triangle") == 0)
        {
            int j;
            double position[3][3], normal[3], diffuse[3][3], specular[3], shininess;
            for (j = 0; j < 3; j++)
            {
                parse_doubles(file, "pos:", position[j]);
                parse_doubles(file, "nor:", normal);
                parse_doubles(file, "dif:", diffuse[j]);
                parse_doubles(file, "spe:", specular);
                parse_shi(file, &shininess);
            }
//			m_obj[m_object_count++] = new Triangle(Vector3(normal[0], normal[1], normal[2]), Vector3(position[0][0], position[0][1], position[0][2]), Vector3(position[1][0], position[1][1], position[1][2]), Vector3(position[2][0], position[2][1], position[2][2]), Vector3(diffuse[0][0], diffuse[0][1], diffuse[0][2]), Vector3(diffuse[1][0], diffuse[1][1], diffuse[1][2]), Vector3(diffuse[2][0], diffuse[2][1], diffuse[2][2]), (float)specular[0], 1.0);
            if (m_object_count == 10000)
            {
                exit(0);
            }
        }
        else if (stricmp(type, "sphere") == 0)
        {
            double position[3], radius, diffuse[3], specular[3], shininess;
            parse_doubles(file, "pos:", position);
            parse_rad(file, &radius);
            parse_doubles(file, "dif:", diffuse);
            parse_doubles(file, "spe:", specular);
//			m_obj[m_object_count++] = new Sphere(Vector3(position[0], position[1], position[2]), radius, Vector3(diffuse[0], diffuse[1], diffuse[2]), specular[0], 1);
            parse_shi(file, &shininess);

            if (m_object_count == 1000)
            {
                exit(0);
            }
        }
        else if (stricmp(type, "light") == 0)
        {
            double pos[3], col[3];
            parse_doubles(file, "pos:", pos);
            parse_doubles(file, "col:", col);

            if (m_light_count == 10)
            {
                printf("too many lights, you should increase MAX_LIGHTS!\n");
                exit(0);
            }
//			m_light[m_light_count++].reset(Vector3((float)pos[0], (float)pos[1], (float)pos[2]), Vector3((float)col[0], (float)col[1], (float)col[2]));
        }
        else
        {
            exit(0);
        }
    }
    return 0;
}

void Help::parse_check(char *expected, char *found)
{
    if (stricmp(expected, found))
    {
        exit(0);
    }

}

void Help::parse_doubles(FILE*file, char *check, double p[3])
{
    char str[100];
    fscanf(file, "%s", str);
    parse_check(check, str);
    fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
}

void Help::parse_rad(FILE*file, double *r)
{
    char str[100];
    fscanf(file, "%s", str);
    parse_check("rad:", str);
    fscanf(file, "%lf", r);
}

void Help::parse_shi(FILE*file, double *shi)
{
    char s[100];
    fscanf(file, "%s", s);
    parse_check("shi:", s);
    fscanf(file, "%lf", shi);
}
