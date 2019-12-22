    #include "HeadersAnalyzer.h"
    
namespace NHeadersAnalyzer
{
    void CHeaders::Add(const char *s, std::size_t n)
    {
        m_s.append(s,n);
    }

    // решил для анализа хедеров использовать stringstream
    // осознаю, что это не эффективно по памяти, но:
    // 1) размер хедеров лимитирован (8К-16К, чаще всего гораздо меньше) 
    // 2) могут существовать некоторые вольности в формате (например, число пробелов Content_Length: <space><space><space><space> 100500), 
    // их удобнее обрабатывать с помощью stringstream
    
    bool CHeaders::BadStatus(std ::string &err)
    {        
        m_gotstatus =true;

        std::stringstream ss(m_s);

        std::string http_version;
        ss >> http_version;
        unsigned int status_code = 0;
        ss >> status_code;

        if (200 != status_code)
        {
            std::string tmp;
            std::getline(ss, tmp);
            err=std::to_string(status_code) + tmp;
            return true;
        }

        return false;
    }

    bool CHeaders::HeadersRecieved(std::string &bodyBytes)
    {
        auto pos = m_s.find("\r\n\r\n", m_searchPos);

        if (pos != std::string::npos)
        {
            bodyBytes = m_s.substr(pos+4);
            return true;
        }

        m_searchPos = m_s.size()-3;
        return false;
    }

    void CHeaders::Analyze()
    {
        std::stringstream ss(m_s);
        m_s.clear();

        std::string header;
        while (std::getline(ss, header) && header != "\r")
        {
            std::string name, value;
            split(header,name,value);
            if (to_upper_copy(name)=="TRANSFER-ENCODING:")
            {
                analyze_transfer_header(value);
                return;
            }
        }
    }
}