/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/core/hardware.h>
#include <et/app/application.h>
#include <et/app/pathresolver.h>
#include <et/locale/locale.hpp>
#include <et/rendering/rendercontext.h>

using namespace et;

void StandardPathResolver::init(const Environment& env)
{
	pushSearchPath(env.applicationPath());
	pushSearchPath(env.applicationInputDataFolder());
}

void StandardPathResolver::validateCaches()
{

	if (Locale::instance().currentLocale() != _cachedLocale)
	{
		_cachedLocale = Locale::instance().currentLocale();
		
		_cachedLang = "." + locale::localeLanguage(_cachedLocale);
		_cachedSubLang = locale::localeSubLanguage(_cachedLocale);
		
		_cachedLanguage = _cachedLang;
		
		if (!_cachedSubLang.empty())
			_cachedLanguage += "-" + _cachedSubLang;
	}
	
    _cachedScreenScaleFactor = currentScreen().scaleFactor;
	_cachedScreenScale = (_cachedScreenScaleFactor > 1) ?
		"@" + floatToStr(_cachedScreenScaleFactor, 2) + "x" : emptyString;
}

std::string StandardPathResolver::resolveFilePath(const std::string& input)
{
	validateCaches();
	
	std::string ext = "." + getFileExt(input);
	std::string name = removeFileExt(getFileName(input));
	std::string path = getFilePath(input);
	
	std::string suggested = input;
	
	std::set<std::string> paths = resolveFolderPaths(path);
	pushSearchPaths(paths);

	bool pathFound = false;
	for (const std::string& folder : _searchPath)
	{
		std::string baseName = folder + name;
		
		if (_cachedScreenScaleFactor > 0)
		{
			// path/file@Sx.ln-sb.ext
			suggested = baseName + _cachedScreenScale + _cachedLanguage + ext;
			if (fileExists(suggested))
			{
				pathFound = true;
				break;
			}

			// path/file@Sx.ln.ext
			suggested = baseName + _cachedScreenScale + _cachedLang + ext;
			if (fileExists(suggested))
			{
				pathFound = true;
				break;
			}

			// path/file@Sx.ext
			suggested = baseName + _cachedScreenScale + ext;
			if (fileExists(suggested))
			{
				pathFound = true;
				break;
			}
		}
		
		// path/file.ln-sb.ext
		suggested = baseName + _cachedLanguage + ext;
		if (fileExists(suggested))
		{
			pathFound = true;
			break;
		}

		// path/file.ln.ext
		suggested = baseName + _cachedLanguage + ext;
		if (fileExists(suggested))
		{
			pathFound = true;
			break;
		}

		if (_cachedScreenScaleFactor > 0)
		{
			// path/file@Sx.ext
			suggested = baseName + _cachedScreenScale + ext;
			if (fileExists(suggested))
			{
				pathFound = true;
				break;
			}
		}
		
		// path/file.ext
		suggested = baseName + ext;
		if (fileExists(suggested))
		{
			pathFound = true;
			break;
		}
	}
	popSearchPaths(paths.size());
	
	if (!_silentErrors && !fileExists(suggested))
		log::warning("Unable to resolve file name: %s", input.c_str());

	return pathFound ? suggested : input;
}

std::set<std::string> StandardPathResolver::resolveFolderPaths(const std::string& input)
{
	validateCaches();

	std::string normalizedInput = normalizeFilePath(input);

	if (!normalizedInput.empty() && normalizedInput.back() == pathDelimiter)
		normalizedInput.pop_back();

	std::set<std::string> result;
	
	std::string suggested = addTrailingSlash(normalizedInput + _cachedLanguage);
	if (folderExists(suggested))
		result.insert(suggested);
	
	suggested = addTrailingSlash(normalizedInput + _cachedLang);
	if (folderExists(suggested))
		result.insert(suggested);
	
	if (folderExists(normalizedInput))
		result.insert(addTrailingSlash(normalizedInput));
	
	for (const std::string& path : _searchPath)
	{
		std::string base = normalizeFilePath(path + normalizedInput);

		if (base.back() == pathDelimiter)
			base.pop_back();
		
		if (_cachedScreenScaleFactor > 0)
		{
			suggested = addTrailingSlash(base + _cachedScreenScale + _cachedLanguage);
			if (fileExists(suggested))
				result.insert(suggested);

			suggested = addTrailingSlash(base + _cachedScreenScale + _cachedLang);
			if (fileExists(suggested))
				result.insert(suggested);

			suggested = addTrailingSlash(base + _cachedScreenScale);
			if (fileExists(suggested))
				result.insert(suggested);
		}

		suggested = addTrailingSlash(base + _cachedLanguage);
		if (folderExists(suggested))
			result.insert(suggested);
		
		suggested = addTrailingSlash(base + _cachedLang);
		if (folderExists(suggested))
			result.insert(suggested);
		
		suggested = addTrailingSlash(base);
		if (folderExists(suggested))
			result.insert(suggested);
	}
	
	return result;
}

std::string StandardPathResolver::resolveFolderPath(const std::string& input)
{
	validateCaches();
	
	std::string suggested = addTrailingSlash(input + _cachedLanguage);
	if (folderExists(suggested))
		return suggested;

	suggested = addTrailingSlash(input + _cachedLang);
	if (folderExists(suggested))
		return suggested;
	
	if (folderExists(input))
		return input;
	
	for (const std::string& path : _searchPath)
	{
		suggested = addTrailingSlash(path + input + _cachedLanguage);
		if (folderExists(suggested))
			return suggested;
		
		suggested = addTrailingSlash(path + input + _cachedLang);
		if (folderExists(suggested))
			return suggested;
		
		suggested = addTrailingSlash(path + input);
		if (folderExists(suggested))
			return suggested;
	}
	
	return input;
}

void StandardPathResolver::pushSearchPath(const std::string& path)
{
	_searchPath.push_front(addTrailingSlash(path));
	normalizeFilePath(_searchPath.front());
}

void StandardPathResolver::pushSearchPaths(const std::set<std::string>& paths)
{
	_searchPath.insert(_searchPath.begin(), paths.begin(), paths.end());
}

void StandardPathResolver::pushRelativeSearchPath(const std::string& path)
{
	const Environment& env = application().environment();
	pushSearchPath(env.applicationPath() + path);

	if (env.applicationPath() != env.applicationInputDataFolder())
	{
		pushSearchPath(env.applicationInputDataFolder() + path);
	}
}

void StandardPathResolver::popSearchPaths(size_t amount)
{
	for (size_t i = 0; i < amount; ++i)
	{
		if (_searchPath.size() > 1)
			_searchPath.pop_front();
	}
}
