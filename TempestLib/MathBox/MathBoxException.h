
#ifndef MATHBOXEXCEPTION_H
#define MATHBOXEXCEPTION_H

class MathBoxException
{
public:
	MathBoxException(const std::string &_message);
	virtual ~MathBoxException(void);
	virtual const char* what(void) const;

private:
	std::string message;
};


#endif
