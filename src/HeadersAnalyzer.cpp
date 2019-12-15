    #include "HeadersAnalyzer.h"
    
namespace NHeadersAnalyzer
{
    void CHeaders::Add(const char *s, std::size_t n)
    {
        m_ss.write(s,n);
    }

    // решил для анализа хедеров использовать stringstream
    // осознаю, что это не эффективно по памяти, но:
    // 1) размер хедеров лимитирован (8К-16К, чаще всего гораздо меньше) 
    // 2) могут существовать некоторые вольности в формате (например, число пробелов Content_Length: <space><space><space><space> 100500), 
    // их удобнее обрабатывать с помощью stringstream
    
    bool CHeaders::BadStatus(std ::string &err)
    {        
        m_gotstatus =true;

        std::string http_version;
        m_ss >> http_version;
        unsigned int status_code = 0;
        m_ss >> status_code;

        if (200 != status_code)
        {
            std::string tmp;
            std::getline(m_ss, tmp);
            err=std::to_string(status_code) + tmp;
            return true;
        }

        return false;
    }

    void CHeaders::analyze()
    {
        std::string header;
        while (std::getline(m_ss, header) && header != "\r")
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

    NFileTypes::FileTypes CHeaders::GetType() const
    {
        if (Chunked_ == m_transfer_encoding)
            return NFileTypes::Chunked_;

        return NFileTypes::Common_;
    }
}