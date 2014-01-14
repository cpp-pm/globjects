#include <vector>
#include <sstream>

#include <glow/Program.h>
#include <glow/logging.h>
#include <glow/StringSource.h>
#include <glow/String.h>
#include <glow/Error.h>
#include <glow/ObjectVisitor.h>
#include <glow/Version.h>

#include <glow/Shader.h>

#include <glow/ref_ptr.h>

#include "IncludeProcessor.h"

namespace
{

std::vector<const char*> collectCStrings(std::vector<std::string> & strings)
{
    std::vector<const char*> cStrings;

    for (const std::string & str : strings)
    {
        cStrings.push_back(str.c_str());
    }

    return cStrings;
}

}

namespace glow
{

Shader::Shader(const GLenum type, StringSource * source)
: Object(create(type))
, m_type(type)
, m_source(nullptr)
, m_compiled(false)
, m_compilationFailed(false)
{
	if (source)
		setSource(source);
}

Shader::Shader(const GLenum type, StringSource * source, const std::vector<std::string> & includePaths)
: Shader(type, source)
{
    setIncludePaths(includePaths);
}

Shader::Shader(const GLenum type)
: Shader(type, nullptr)
{
}

Shader * Shader::fromString(const GLenum type, const std::string & sourceString)
{
    return new Shader(type, new String(sourceString));
}

Shader::~Shader()
{
	if (m_source)
	{
		m_source->deregisterListener(this);
	}

	if (ownsGLObject())
	{
		glDeleteShader(m_id);
		CheckGLError();
	}
}

GLuint Shader::create(GLenum type)
{
	GLuint result = glCreateShader(type);
	CheckGLError();
	return result;
}

void Shader::accept(ObjectVisitor& visitor)
{
	visitor.visitShader(this);
}

GLenum Shader::type() const
{
	return m_type;
}

void Shader::setSource(StringSource * source)
{
    if (source == m_source)
        return;

	if (m_source)
		m_source->deregisterListener(this);

	m_source = source;

	if (m_source)
		m_source->registerListener(this);

	updateSource();
}

void Shader::setSource(const std::string & source)
{
    setSource(new String(source));
}

const StringSource* Shader::source() const
{
	return m_source;
}

void Shader::notifyChanged()
{
	updateSource();
}

void Shader::updateSource()
{
    if (m_source)
    {
        std::vector<std::string> sources;

        if (!GLEW_ARB_shading_language_include || Version::current() < Version(3, 2)) // fallback
        {
            ref_ptr<StringSource> resolvedSource = IncludeProcessor::resolveIncludes(m_source);

            sources = resolvedSource->strings();
        }
        else
        {
            sources = m_source->strings();
        }

        std::vector<const char*> cStrings = collectCStrings(sources);

        glShaderSource(m_id, static_cast<GLint>(cStrings.size()), cStrings.data(), nullptr);
    }
    else
    {
        glShaderSource(m_id, 0, nullptr, nullptr);
    }

    invalidate();
}

void Shader::setIncludePaths(const std::vector<std::string> & includePaths)
{
    m_includePaths = includePaths;

    invalidate();
}

bool Shader::compile()
{
    if (m_compilationFailed)
        return false;

    if (glCompileShaderIncludeARB && Version::current() >= Version(3, 2))
    {
        std::vector<const char*> cStrings = collectCStrings(m_includePaths);
        glCompileShaderIncludeARB(m_id, static_cast<GLint>(cStrings.size()), cStrings.data(), nullptr);
        CheckGLError();
    }
    else
    {
        glCompileShader(m_id);
        CheckGLError();
    }

    m_compiled = checkCompileStatus();

    m_compilationFailed = !m_compiled;

    changed();

    return m_compiled;
}

bool Shader::isCompiled() const
{
	return m_compiled;
}

void Shader::invalidate()
{
    m_compiled = false;
    m_compilationFailed = false;
    changed();
}

GLint Shader::get(GLenum pname) const
{
    GLint value = 0;
    glGetShaderiv(m_id, pname, &value);
    CheckGLError();

    return value;
}

bool Shader::checkCompileStatus() const
{
    GLint status = get(GL_COMPILE_STATUS);

    if (GL_FALSE == status)
    {
        critical()
            << "Compiler error:" << std::endl
            << shaderString() << std::endl
            << infoLog();

        return false;
    }

    return true;
}

std::string Shader::infoLog() const
{
    GLsizei length = get(GL_INFO_LOG_LENGTH);
	std::vector<char> log(length);

	glGetShaderInfoLog(m_id, length, &length, log.data());
	CheckGLError();

	return std::string(log.data(), length);
}

std::string Shader::shaderString() const
{
	std::stringstream ss;

	ss << "Shader(" << typeString();

    std::string shortInfo = m_source->shortInfo();
    if (shortInfo.size() > 0)
        ss << ", " << shortInfo;

	ss << ")";

	return ss.str();
}

std::string Shader::typeString() const
{
	switch (m_type)
	{
	case GL_GEOMETRY_SHADER:
		return "GL_GEOMETRY_SHADER";
	case GL_FRAGMENT_SHADER:
		return "GL_FRAGMENT_SHADER";
	case GL_VERTEX_SHADER:
		return "GL_VERTEX_SHADER";
	case GL_TESS_EVALUATION_SHADER:
		return "GL_TESS_EVALUATION_SHADER";
	case GL_TESS_CONTROL_SHADER:
		return "GL_TESS_CONTROL_SHADER";
	case GL_COMPUTE_SHADER:
		return "GL_COMPUTE_SHADER";
	default:
		return "Unknown Shader Type";
	}
}

} // namespace glow
