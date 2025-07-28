import json
import re
from urllib.parse import quote
import argparse
import subprocess
import sys
import os

def get_redirect_url(query):
    script_dir = os.path.dirname(__file__)
    json_path = os.path.join(script_dir, 'source.json')
    try:
        with open(json_path, 'r') as f:
            bangs_list = json.load(f)
    except FileNotFoundError:
        print(f"Error: {json_path} not found.", file=sys.stderr)
        return None
    except json.JSONDecodeError:
        print(f"Error: Could not decode JSON from {json_path}.", file=sys.stderr)
        return None

    bangs = {b["t"]: b for b in bangs_list}

    match = re.match(r"^!([a-zA-Z0-9]+)\s*(.*)$", query)

    bang_candidate = None
    clean_query = query

    if match:
        bang_candidate = match.group(1).lower()
        clean_query = match.group(2) or ""

    selected_bang = None
    if bang_candidate:
        selected_bang = bangs.get(bang_candidate)

    if not selected_bang:
        selected_bang = bangs.get("ddg")
        if bang_candidate and match:
            clean_query = query

    if not selected_bang:
        return None

    if not clean_query:
        return f"https://{selected_bang['d']}"

    final_url = selected_bang["u"].replace("{{{s}}}", quote(clean_query, safe=""))
    return final_url

def main():
    parser = argparse.ArgumentParser(description="resolve a string to a URL and either open it or print it.")
    parser.add_argument("query", help="The search query or bang command.")
    parser.add_argument("--open", "-o", action="store_true", help="Open the URL using xdg-open.")

    args = parser.parse_args()

    redirect_url = get_redirect_url(args.query)

    if redirect_url:
        if args.open:
            try:
                subprocess.run(["xdg-open", redirect_url], check=True)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"Error opening URL with xdg-open: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            print(redirect_url)
    else:
        print("Could not determine a redirect URL.", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
