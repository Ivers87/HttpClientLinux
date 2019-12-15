#include <string>

class CUrlParser 
{
public:
    CUrlParser(const std::string& url, bool needFilename = true):
       m_port(80)
	{
        parse(url, needFilename);
	}

	std::string GetHost()const {return m_host;}
	std::string GetPath()const {return m_path;}
	std::string GetFilename()const {return m_file;}
	int GetPort()const {return m_port;}
		
private:
    void parse(const std::string& url, bool needFilename)
    {
        auto pos_prot = url.find("://");
        unsigned pos_hst = (std::string::npos == pos_prot) ? 0 : pos_prot + 3;

        auto pos_slash = url.find("/", pos_hst);

        if (std::string::npos == pos_slash)
        {
            setHostAndPort(url.substr(pos_hst));
            return;
        }

        setHostAndPort(url.substr(pos_hst, pos_slash - pos_hst));

        auto pos_query = url.find("?", pos_slash);

        m_path = (std::string::npos == pos_query) ? url.substr(pos_slash) : url.substr(pos_slash, pos_query - pos_slash);
        
        if (needFilename)
            m_file = getFile(m_path);
    }
private:

    static std::string getFile(const std::string& s)
    {
        return s.substr(s.find_last_of("/") + 1);
    }

    void setHostAndPort(const std::string& url)
    {
        auto pos_dv = url.find(":");

        if (std::string::npos == pos_dv)
        {
            m_host = url;
            return;
        }

        m_host = url.substr(0, pos_dv);
        setPort(url.substr(pos_dv + 1));
    }

    void setPort(const std::string& sport)
    {
        try
        {
            m_port = std::stoi(sport);
        }
        catch (...)
        {
        }
    }

private:
	std::string  m_host, m_path, m_file;
    int m_port;
};
