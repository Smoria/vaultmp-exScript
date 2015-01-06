#ifndef FUNC_H
#define FUNC_H

bool FileExists(const char *fname) noexcept // Check existence of a file
{
	return access(fname, 0) != -1;
}

std::string string_replace(std::string src, std::string const& target, std::string const& repl) noexcept// Return a string with all occurrences of substring target replaced by repl.
{
	// handle error situations/trivial cases
	if (target.length() == 0)
	{
		// searching for a match to the empty string will result in
		//  an infinite loop
		//  it might make sense to throw an exception for this case
		return src;
	}
	if (src.length() == 0)
	{
		return src;  // nothing to match against
	}
	size_t idx = 0;
	for (;;)
	{
		idx = src.find(target, idx);
		if (idx == std::string::npos)  break;
		src.replace(idx, target.length(), repl);
		idx += repl.length();
	}
	return src;
}

std::string GetDeathReason(Death d, ID killer) noexcept // Return death message for different reasons
{
	switch (d)
	{
	case Death::None:
		return "died";
	case Death::Explosion:
		return "blew up";
	case Death::Gun:
		return "was shot by " + GetBaseName(killer);
	case Death::BluntWeapon:
		return "was slain by " + GetBaseName(killer);
	case Death::HandToHand:
		return "was torn in half by " + GetBaseName(killer);
	case Death::ObjectImpact:
		return "skull was crushed";
	case Death::Poison:
		return "died from poison";
	case Death::Radiation:
		return "was turned into pile of flesh";
	}
	return "died from unknown reason";
}
#endif